/* Copyright (C) 2010 - 2012 Sergei Golubchik and Monty Program Ab

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public
   License along with this library; if not see <http://www.gnu.org/licenses>
   or write to the Free Software Foundation, Inc., 
   51 Franklin St., Fifth Floor, Boston, MA 02110, USA */

/**
  @file
  
  Support code for the client side (libmariadb) plugins

  Client plugins are somewhat different from server plugins, they are simpler.

  They do not need to be installed or in any way explicitly loaded on the
  client, they are loaded automatically on demand.
  One client plugin per shared object, soname *must* match the plugin name.

  There is no reference counting and no unloading either.
*/

#if _MSC_VER
/* Silence warnings about variable 'unused' being used. */
#define FORCE_INIT_OF_VARS 1
#endif

#include <my_global.h>
#include <my_sys.h>
#include <ma_common.h> 
#include <m_string.h>
#ifdef THREAD
#include <my_pthread.h>
#else
#include <my_no_pthread.h>
#endif

#include "errmsg.h"
#include <mysql/client_plugin.h>

#ifndef _WIN32
#include <dlfcn.h>
#endif

struct st_client_plugin_int {
  struct st_client_plugin_int *next;
  void   *dlhandle;
  struct st_mysql_client_plugin *plugin;
};

static my_bool initialized= 0;
static MEM_ROOT mem_root;

#define plugin_declarations_sym "_mysql_client_plugin_declaration_"

static uint plugin_version[MYSQL_CLIENT_MAX_PLUGINS]=
{
  MYSQL_CLIENT_DB_PLUGIN_INTERFACE_VERSION, /* these two are taken by Connector/C */
  0, /* these two are taken by Connector/C */
  MYSQL_CLIENT_AUTHENTICATION_PLUGIN_INTERFACE_VERSION
};

/*
  Loaded plugins are stored in a linked list.
  The list is append-only, the elements are added to the head (like in a stack).
  The elements are added under a mutex, but the list can be read and traversed
  without any mutex because once an element is added to the list, it stays
  there. The main purpose of a mutex is to prevent two threads from
  loading the same plugin twice in parallel.
*/
struct st_client_plugin_int *plugin_list[MYSQL_CLIENT_MAX_PLUGINS];
#ifdef THREAD
static pthread_mutex_t LOCK_load_client_plugin;
#endif

static int is_not_initialized(MYSQL *mysql, const char *name)
{
  if (initialized)
    return 0;

  my_set_error(mysql, CR_AUTH_PLUGIN_CANNOT_LOAD,
               SQLSTATE_UNKNOWN, ER(CR_AUTH_PLUGIN_CANNOT_LOAD),
               name, "not initialized");
  return 1;
}


/**
  finds a plugin in the list

  @param name   plugin name to search for
  @param type   plugin type

  @note this does NOT necessarily need a mutex, take care!
  
  @retval a pointer to a found plugin or 0
*/

static struct st_mysql_client_plugin *find_plugin(const char *name, int type)
{
  struct st_client_plugin_int *p;

  DBUG_ASSERT(initialized);
  DBUG_ASSERT(type >= 0 && type < MYSQL_CLIENT_MAX_PLUGINS);
  if (type < 0 || type >= MYSQL_CLIENT_MAX_PLUGINS)
    return 0;

  for (p= plugin_list[type]; p; p= p->next)
  {
    if (strcmp(p->plugin->name, name) == 0)
      return p->plugin;
  }
  return NULL;
}


/**
  verifies the plugin and adds it to the list

  @param mysql          MYSQL structure (for error reporting)
  @param plugin         plugin to install
  @param dlhandle       a handle to the shared object (returned by dlopen)
                        or 0 if the plugin was not dynamically loaded
  @param argc           number of arguments in the 'va_list args'
  @param args           arguments passed to the plugin initialization function

  @retval a pointer to an installed plugin or 0
*/

static struct st_mysql_client_plugin *
add_plugin(MYSQL *mysql, struct st_mysql_client_plugin *plugin, void *dlhandle,
           int argc, va_list args)
{
  const char *errmsg;
  struct st_client_plugin_int plugin_int, *p;
  char errbuf[1024];

  DBUG_ASSERT(initialized);

  plugin_int.plugin= plugin;
  plugin_int.dlhandle= dlhandle;

  if (plugin->type >= MYSQL_CLIENT_MAX_PLUGINS)
  {
    errmsg= "Unknown client plugin type";
    goto err1;
  }

  if (plugin->interface_version < plugin_version[plugin->type] ||
      (plugin->interface_version >> 8) >
       (plugin_version[plugin->type] >> 8))
  {
    errmsg= "Incompatible client plugin interface";
    goto err1;
  }

  /* Call the plugin initialization function, if any */
  if (plugin->init && plugin->init(errbuf, sizeof(errbuf), argc, args))
  {
    errmsg= errbuf;
    goto err1;
  }

  p= (struct st_client_plugin_int *)
    memdup_root(&mem_root, (char *)&plugin_int, sizeof(plugin_int));

  if (!p)
  {
    errmsg= "Out of memory";
    goto err2;
  }

#ifdef THREAD
  safe_mutex_assert_owner(&LOCK_load_client_plugin);
#endif

  p->next= plugin_list[plugin->type];
  plugin_list[plugin->type]= p;

  return plugin;

err2:
  if (plugin->deinit)
    plugin->deinit();
err1:
  if (dlhandle)
    (void)dlclose(dlhandle);
  my_set_error(mysql, CR_AUTH_PLUGIN_CANNOT_LOAD, SQLSTATE_UNKNOWN,
               ER(CR_AUTH_PLUGIN_CANNOT_LOAD), plugin->name, errmsg);
  return NULL;
}


/**
  Loads plugins which are specified in the environment variable
  LIBMYSQL_PLUGINS.
  
  Multiple plugins must be separated by semicolon. This function doesn't
  return or log an error.

  The function is be called by mysql_client_plugin_init

  @todo
  Support extended syntax, passing parameters to plugins, for example
  LIBMYSQL_PLUGINS="plugin1(param1,param2);plugin2;..."
  or
  LIBMYSQL_PLUGINS="plugin1=int:param1,str:param2;plugin2;..."
*/

static void load_env_plugins(MYSQL *mysql)
{
  char *plugs, *free_env, *s= getenv("LIBMYSQL_PLUGINS");

  /* no plugins to load */
  if (!s)
    return;

  free_env= plugs= my_strdup(s, MYF(MY_WME));

  do {
    if ((s= strchr(plugs, ';')))
      *s= '\0';
    mysql_load_plugin(mysql, plugs, -1, 0);
    plugs= s + 1;
  } while (s);

  my_free(free_env);
}

/********** extern functions to be used by libmariadb *********************/

/**
  Initializes the client plugin layer.

  This function must be called before any other client plugin function.

  @retval 0    successful
  @retval != 0 error occured
*/

int mysql_client_plugin_init()
{
  MYSQL mysql;
  struct st_mysql_client_plugin **builtin;
  va_list unused;
  LINT_INIT_STRUCT(unused);

  if (initialized)
    return 0;

  bzero(&mysql, sizeof(mysql)); /* dummy mysql for set_mysql_extended_error */

  pthread_mutex_init(&LOCK_load_client_plugin, MY_MUTEX_INIT_SLOW);
  init_alloc_root(&mem_root, 128, 128);

  bzero(&plugin_list, sizeof(plugin_list));

  initialized= 1;

  pthread_mutex_lock(&LOCK_load_client_plugin);

  for (builtin= mysql_client_builtins; *builtin; builtin++)
    add_plugin(&mysql, *builtin, 0, 0, unused);
  pthread_mutex_unlock(&LOCK_load_client_plugin);

  load_env_plugins(&mysql);

  return 0;
}


/**
  Deinitializes the client plugin layer.

  Unloades all client plugins and frees any associated resources.
*/

void mysql_client_plugin_deinit()
{
  int i;
  struct st_client_plugin_int *p;

  if (!initialized)
    return;

  for (i=0; i < MYSQL_CLIENT_MAX_PLUGINS; i++)
    for (p= plugin_list[i]; p; p= p->next)
    {
      if (p->plugin->deinit)
        p->plugin->deinit();
      if (p->dlhandle)
        (void)dlclose(p->dlhandle);
    }

  bzero(&plugin_list, sizeof(plugin_list));
  initialized= 0;
  free_root(&mem_root, MYF(0));
  pthread_mutex_destroy(&LOCK_load_client_plugin);
}

/************* public facing functions, for client consumption *********/

/* see <mysql/client_plugin.h> for a full description */
struct st_mysql_client_plugin * STDCALL
mysql_client_register_plugin(MYSQL *mysql,
                             struct st_mysql_client_plugin *plugin)
{
  va_list unused;
  LINT_INIT_STRUCT(unused);

  if (is_not_initialized(mysql, plugin->name))
    return NULL;

  pthread_mutex_lock(&LOCK_load_client_plugin);

  /* make sure the plugin wasn't loaded meanwhile */
  if (find_plugin(plugin->name, plugin->type))
  {
    my_set_error(mysql, CR_AUTH_PLUGIN_CANNOT_LOAD,
                 SQLSTATE_UNKNOWN, ER(CR_AUTH_PLUGIN_CANNOT_LOAD),
                 plugin->name, "it is already loaded");
    plugin= NULL;
  }
  else
    plugin= add_plugin(mysql, plugin, 0, 0, unused);

  pthread_mutex_unlock(&LOCK_load_client_plugin);
  return plugin;
}


/* see <mysql/client_plugin.h> for a full description */
struct st_mysql_client_plugin * STDCALL
mysql_load_plugin_v(MYSQL *mysql, const char *name, int type,
                    int argc, va_list args)
{
  const char *errmsg;
#ifdef _WIN32
  char errbuf[255];
#endif
  char dlpath[FN_REFLEN+1];
  void *sym, *dlhandle;
  struct st_mysql_client_plugin *plugin;
  char *env_plugin_dir= getenv("MARIADB_PLUGIN_DIR");

  if (is_not_initialized(mysql, name))
    return NULL;

  pthread_mutex_lock(&LOCK_load_client_plugin);

  /* make sure the plugin wasn't loaded meanwhile */
  if (type >= 0 && find_plugin(name, type))
  {
    errmsg= "it is already loaded";
    goto err;
  }

  /* Compile dll path */
  strxnmov(dlpath, sizeof(dlpath) - 1,
           mysql->options.extension && mysql->options.extension->plugin_dir ?
           mysql->options.extension->plugin_dir : (env_plugin_dir) ? env_plugin_dir :
           PLUGINDIR, "/",
           name, SO_EXT, NullS);
   
  /* Open new dll handle */
  if (!(dlhandle= dlopen((const char *)dlpath, RTLD_NOW)))
  {
#ifdef _WIN32
   FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                 NULL,
                 GetLastError(),
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                 (LPTSTR)&errbuf, 255, NULL);
   errmsg= errbuf;
#else
    errmsg= dlerror();
#endif
    goto err;
  }

  if (!(sym= dlsym(dlhandle, plugin_declarations_sym)))
  {
    errmsg= "not a plugin";
    (void)dlclose(dlhandle);
    goto err;
  }

  plugin= (struct st_mysql_client_plugin*)sym;

  if (type >=0 && type != plugin->type)
  {
    errmsg= "type mismatch";
    goto err;
  }

  if (strcmp(name, plugin->name))
  {
    errmsg= "name mismatch";
    goto err;
  }

  if (type < 0 && find_plugin(name, plugin->type))
  {
    errmsg= "it is already loaded";
    goto err;
  }

  plugin= add_plugin(mysql, plugin, dlhandle, argc, args);

  pthread_mutex_unlock(&LOCK_load_client_plugin);

  return plugin;

err:
  pthread_mutex_unlock(&LOCK_load_client_plugin);
  my_set_error(mysql, CR_AUTH_PLUGIN_CANNOT_LOAD, SQLSTATE_UNKNOWN,
               ER(CR_AUTH_PLUGIN_CANNOT_LOAD), name, errmsg);
  return NULL;
}


/* see <mysql/client_plugin.h> for a full description */
struct st_mysql_client_plugin * STDCALL
mysql_load_plugin(MYSQL *mysql, const char *name, int type, int argc, ...)
{
  struct st_mysql_client_plugin *p;
  va_list args;
  va_start(args, argc);
  p= mysql_load_plugin_v(mysql, name, type, argc, args);
  va_end(args);
  return p;
}


/* see <mysql/client_plugin.h> for a full description */
struct st_mysql_client_plugin * STDCALL
mysql_client_find_plugin(MYSQL *mysql, const char *name, int type)
{
  struct st_mysql_client_plugin *p;

  if (is_not_initialized(mysql, name))
    return NULL;

  if (type < 0 || type >= MYSQL_CLIENT_MAX_PLUGINS)
  {
    my_set_error(mysql, CR_AUTH_PLUGIN_CANNOT_LOAD, SQLSTATE_UNKNOWN,
                 ER(CR_AUTH_PLUGIN_CANNOT_LOAD), name, "invalid type");
  }

  if ((p= find_plugin(name, type)))
    return p;

  /* not found, load it */
  return mysql_load_plugin(mysql, name, type, 0);
}


