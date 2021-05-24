#-*- coding: utf-8 -*-
import sys, os, time, locale

"""
Metin2 Windows Server Files Folder Control Script 2.0 / 2015
Koray
Metin2dev - Turkmmo

*Start commands: "oyun", "oyunuac", "ac", "oyunu ac" , "oyunuac", "start", "game"
-Arg1: command - Arg2: Type[debug-normal] - Arg3: Channel count
-Example: m2panel.py start debug 1
*Clean commands: "temizle", "temiz", "log", "clear"
-Arg1: command
-Example: m2panel.py clear
*Remove commands: "kaldir", "remove", "sil"
-Arg1: command - Arg2: Type[debug-normal]
-Example: m2panel.py remove debug
*Install commands: "kur", "install", "yukle"
-Arg1: command - Arg2: Type[debug-normal]
-Example: m2panel.py install debug
*Backup command: "yedekal", "backup"
-Arg1: command
-Example: m2panel.py backup
*Install backup command: "yedekkur", "installbackup", "reinstall"
-Arg1: command
-Example: m2panel.py installbackup
"""

class LOCALE(object):
	def __new__(self, x):
		self.WORD_DICT = {
			"Tanimsiz Islem": "Undefined process", 
			"Oyun baslatma islemi baslatildi": "Game startup process has initiated",
			"baslatildi": "started",
			"Oyun baslatma islemi tamamlandi": "Game startup process has completed",
			"Log temizleme baslatildi": "Log clean process has initiated",
			"dosyasi silindi": "file removed",
			"dosyasi bulunamadi": "file not found",
			"Log temizleme tamamlandi": "Log clean process has completed",
			"Silme Islemi baslatildi": "Uninstall process has initiated",
			"klasoru silindi": "folder removed",
			"klasoru bulunamadi": "folder not found",
			"Silme islemi tamamlandi": "Uninstall process has completed",
			"Symlink kurulum baslatildi": "Symlink setup process has initiated",
			"dizininde symlink olusturuldu": "symlink created in this folder",
			"dosyasinda symlink olusturuldu": "symlink created in this file",
			"Symlink kurulum tamamlandi": "Symlink setup process has completed",
			"Yedek alma islemi baslatildi": "Backup process has initiated",
			"Yedek alma islemi tamamlandi": "Backup process has completed",
			"yedek alindi": "backup created",
			"Lutfen yedek Secin": "Please select backup",
			"Dosyalari eskileri ile degistirmek icin: Y\nDosyalarin Tumunu Degistirmek Icin: A\nGeri Donmek Icin: Q": "To modify files with the old ones: Y\nTo replace all of the file: A\nTo return to Q"
		}
		if locale.getdefaultlocale()[0] == "tr_TR": return x
		else: return self.WORD_DICT[x]

class BACKUP(object):
	def __new__(self, rardizin, yedekalincak):
		os.system("%s a -ag+YYYYMMDD-HHMMSS _backup %s" % (rardizin, yedekalincak))

class M2FilesHelper:
	def sys_argc(self): return int(len(sys.argv)) - 1
	def sys_argv(self): return sys.argv[1:]
	def __init__(self):		
		self.tip = None
		self.KanalSayisi = 0
		self.Environment = ""
		self.dizin = os.getcwd()
		
		if os.path.isfile(self.dizin+"\\syserr.txt"):
			os.remove(self.dizin+"\\syserr.txt")
		if os.path.isfile(self.dizin+"\\syslog.txt"):
			os.remove(self.dizin+"\\syslog.txt")
		
		if not self.sys_argc():
			self.usage()
			return
			
		print("Arg count: %d List: %s" % (self.sys_argc(), self.sys_argv()))
		
		if self.sys_argc() >= 2:
			self.TurSec(self.sys_argv()[1])
		if self.sys_argc() == 3:
			self.KanalSayisi = self.sys_argv()[2]
			self.Environment = self.sys_argv()[2]
		
		self.IslemSec(self.sys_argv()[0])
	
	def sys_log(self, log):
		x=open("syslog.txt", "a")
		x.write("SYSLOG %s: %s\n" % (str(time.strftime("%H.%M.%S - %d.%m.%Y")), str(log)))
		x.close()
	def sys_err(self, log):
		x=open("syserr.txt", "a")
		x.write("SYSERR %s: %s\n" % (str(time.strftime("%H.%M.%S - %d.%m.%Y")), str(log)))
		x.close()
	
	def IslemSec(self, islem):
		if islem in ["oyun", "oyunuac", "ac", "oyunu ac" , "oyunuac", "start", "game"]:
			self.Baslat()
		elif islem in ["temizle", "temiz", "log", "clear"]:
			self.Temizle()
		elif islem in ["kaldir", "remove", "sil"]:
			self.Kaldir()
		elif islem in ["kur", "install", "yukle"]:
			self.SymLinkKur()
		elif islem in ["yedekal", "backup"]:
			self.Yedekal()
		elif islem in ["yedekkur", "installbackup", "reinstall"]:
			self.Yedekkur()
		else:
			print LOCALE("Tanimsiz Islem")
			return
	
	def TurSec(self, tur):
		tur = str(tur)
		if tur in ["normal", "n"]:
			self.tip = "normal"
		elif tur in ["debug", "d"]:
			self.tip = "debug"
		else:
			print LOCALE("Tanimsiz Islem")
			return		
	
	def Baslat(self):
		print LOCALE("Oyun baslatma islemi baslatildi")
		self.KanalSayisi = int(self.KanalSayisi)
		if self.KanalSayisi < 1:
			print LOCALE("Tanimsiz Islem")
			return

		container_roots = ["\\auth\\", "\\db\\", "\\game99\\"]
		for i in xrange(self.KanalSayisi):
			container_roots.append("\\kanal\\kanal%s\\" % str(i + 1))
			
		if self.tip == "normal":
			container_core = ["game.exe", "db.exe"]
		if self.tip == "debug":
			container_core = ["game_d.exe", "db_d.exe"]
		
		for i in container_roots:
			if str(i) == "\\db\\":
				os.chdir(str(self.dizin)+ str(i))
				os.startfile(str(self.dizin) + str(i) + container_core[1])
				self.sys_log("'%s' %s" % (str(self.dizin) + str(i) + container_core[1], LOCALE("baslatildi")))
			else:
				os.chdir(str(self.dizin)+ str(i))
				os.startfile(str(self.dizin) + str(i) + container_core[0])
				self.sys_log("'%s' %s" % (str(self.dizin) + str(i) + container_core[0], LOCALE("baslatildi")))
		
		print LOCALE("Oyun baslatma islemi tamamlandi")
		
	def Temizle(self):
		print LOCALE("Log temizleme baslatildi")
		garbage_container_roots = ["\\auth\\", "\\db\\", "\\game99\\", "\\kanal\\"]
		garbage_container_log = [
			"syserr.txt", "syslog.txt", "neterr.txt", "PTS.txt", "usage.txt", "packet_info.txt", "VERSION.txt",
			"*.dmp", "mob_count", "p2p_packet_info.txt", "DEV_LOG.log"
		]
		
		for i in garbage_container_roots:
			for k in xrange(len(garbage_container_log)):
				try:
					os.remove(self.dizin + str(i) + str(garbage_container_log[k]))
					self.sys_log("'%s' %s" % (str(self.dizin) + str(i) + str(garbage_container_log[k]), LOCALE("dosyasi silindi")))
				except:
					self.sys_err("'%s' %s" % (str(self.dizin) + str(i) + str(garbage_container_log[k]), LOCALE("dosyasi bulunamadi")))
					pass
					
		print LOCALE("Log temizleme tamamlandi")
	
	def Kaldir(self):
		print LOCALE("Silme Islemi baslatildi")
		garbage_container_roots = ["\\auth\\", "\\db\\", "\\game99\\", "\\kanal\\"]
		garbage_container_dir = ["data", "locale", "log", "package", "mark"]
		if self.tip == "debug":
			garbage_container_file = ["db_d.exe", "game_d.exe", "DevIL.dll"]
		elif self.tip == "normal":
			garbage_container_file = ["db.exe", "game.exe", "DevIL.dll"]
		else:
			print LOCALE("Tanimsiz Islem")
			return	
		garbage_container_config = [
			"\\ayar\\proto\\mob_proto.txt","\\ayar\\proto\\mob_names.txt", "\\ayar\\proto\\item_names.txt", "\\ayar\\proto\\item_proto.txt",
			"\\ayar\\proto\\mob_proto2.txt","\\ayar\\proto\\mob_proto_edit.txt","\\ayar\\proto\\mob_names2.txt", "\\ayar\\proto\\item_names2.txt", "\\ayar\\proto\\item_proto2.txt",
			"\\db\\mob_proto.txt","\\db\\mob_names.txt", "\\db\\item_names.txt", "\\db\\item_proto.txt",
			"\\db\\mob_proto2.txt","\\db\\mob_proto_edit.txt","\\db\\mob_names2.txt", "\\db\\item_names2.txt", "\\db\\item_proto2.txt",
			"\\db\\object_proto.txt", "\\ayar\\proto\\object_proto.txt",
			"\\ayar\\kanal1_config.txt", "\\ayar\\game99_config.txt", "\\ayar\\auth_config.txt", "\\ayar\\db_conf.txt"
		]
		
		for i in garbage_container_roots:
			for k in xrange(len(garbage_container_dir)):
				try:
					os.rmdir(self.dizin + str(i) + str(garbage_container_dir[k]))
					self.sys_log("'%s' %s" % (str(self.dizin) + str(i) + str(garbage_container_dir[k]), LOCALE("klasoru silindi")))
				except:
					self.sys_err("'%s' %s" % (str(self.dizin) + str(i) + str(garbage_container_dir[k]), LOCALE("klasoru bulunamadi")))
					pass
				
			for x in xrange(len(garbage_container_file)):
				try:
					os.remove(self.dizin + str(i) + str(garbage_container_file[x]))
					self.sys_log("'%s' %s" % (str(self.dizin) + str(i) + str(garbage_container_file[x]), LOCALE("dosyasi silindi")))
				except:
					self.sys_err("'%s' %s" % (str(self.dizin) + str(i) + str(garbage_container_file[x]), LOCALE("dosyasi bulunamadi")))
					pass
		
		for i in garbage_container_config:
			try:
				os.remove(self.dizin + str(i))
				self.sys_log("'%s' %s" % (str(self.dizin) + str(i), LOCALE("dosyasi silindi")))
			except:		
				self.sys_err("'%s' %s" % (str(self.dizin) + str(i), LOCALE("dosyasi bulunamadi")))
				pass
		
		print LOCALE("Silme islemi tamamlandi")
	
	def SymLinkKur(self):
		print LOCALE("Symlink kurulum baslatildi")
		symlink_targets_dir = [
			["kanal\\package", "..\\..\\Server\\share\\package"], ["kanal\\locale", "..\\..\\Server\\share\\locale"], ["kanal\\data", "..\\..\\Server\\share\\data"], ["kanal\\log", "..\\..\\Server\\share\\log"], ["kanal\\mark", "..\\..\\Server\\share\\mark"],
			["auth\\package",  "..\\..\\Server\\share\\package"], ["auth\\locale", 	"..\\..\\Server\\share\\locale"], ["auth\\data",  "..\\..\\Server\\share\\data"], ["auth\\log",  "..\\..\\Server\\share\\auth\\log"], ["auth\\mark", "..\\..\\Server\\share\\mark"],
			["db\\package", "..\\..\\Server\\share\\package"], ["db\\locale", "..\\..\\Server\\share\\locale"], ["db\\data", "..\\..\\Server\\share\\data"], ["db\\log", "..\\..\\Server\\share\\db\\log"],
			["game99\\package", "..\\..\\Server\\share\\package"], ["game99\\locale", "..\\..\\Server\\share\\locale"], ["game99\\data", "..\\..\\Server\\share\\data"], ["game99\\log", "..\\..\\Server\\share\\game99\\log"]
		]
		
		if self.tip == "debug":
			symlink_targets_file = [
				["db\\db_d.exe", "..\\bin\\db_cache.exe"], ["auth\\game_d.exe", "..\\bin\\game_server.exe"], ["game99\\game_d.exe", "..\\bin\\game_server.exe"],
				["kanal\\game_d.exe", "..\\bin\\game_server.exe"],

				["db\\item_names.txt", "..\\..\\Server\\share\\conf\\item_names.txt"], ["db\\mob_names.txt", "..\\..\\Server\\share\\conf\\mob_names.txt"],
				["db\\item_names2.txt", "..\\..\\Server\\share\\conf\\item_names2.txt"], ["db\\mob_names2.txt", "..\\..\\Server\\share\\conf\\mob_names2.txt"],
				["db\\item_proto.txt", "..\\..\\Server\\share\\conf\\item_proto.txt"], ["db\\mob_proto.txt", "..\\..\\Server\\share\\conf\\mob_proto.txt"],
				["db\\item_proto2.txt", "..\\..\\Server\\share\\conf\\item_proto2.txt"], ["db\\mob_proto2.txt", "..\\..\\Server\\share\\conf\\mob_proto2.txt"],
				["db\\mob_proto_edit.txt", "..\\..\\Server\\share\\conf\\mob_proto_edit.txt"],
				["db\\object_proto.txt", "..\\..\\Server\\share\\conf\\object_proto.txt"],

				["ayar\\auth_config.txt", "auth\\CONFIG"], ["ayar\\game99_config.txt", "game99\\CONFIG"],
				["ayar\\kanal1_config.txt", "kanal\\CONFIG"], ["ayar\\db_conf.txt", "db\\CONFIG"],
			]
		elif self.tip == "normal":
			symlink_targets_file = [
				["db\\db.exe", "..\\bin\\db_cache.exe"], ["auth\\game.exe", "..\\bin\\game_server.exe"], ["game99\\game.exe", "..\\bin\\game_server.exe"],
				["kanal\\game.exe", "..\\bin\\game_server.exe"],	

				["db\\item_names.txt", "..\\..\\Server\\share\\conf\\item_names.txt"], ["db\\mob_names.txt", "..\\..\\Server\\share\\conf\\mob_names.txt"],
				["db\\item_names2.txt", "..\\..\\Server\\share\\conf\\item_names2.txt"], ["db\\mob_names2.txt", "..\\..\\Server\\share\\conf\\mob_names2.txt"],
				["db\\item_proto.txt", "..\\..\\Server\\share\\conf\\item_proto.txt"], ["db\\mob_proto.txt", "..\\..\\Server\\share\\conf\\mob_proto.txt"],
				["db\\item_proto2.txt", "..\\..\\Server\\share\\conf\\item_proto2.txt"], ["db\\mob_proto2.txt", "..\\..\\Server\\share\\conf\\mob_proto2.txt"],
				["db\\mob_proto_edit.txt", "..\\..\\Server\\share\\conf\\mob_proto_edit.txt"],
				["db\\object_proto.txt", "..\\..\\Server\\share\\conf\\object_proto.txt"],

				["ayar\\auth_config.txt", "auth\\CONFIG"], ["ayar\\game99_config.txt", "game99\\CONFIG"],
				["ayar\\kanal1_config.txt", "kanal\\CONFIG"], ["ayar\\db_conf.txt", "db\\CONFIG"],
			]
		else:
			print LOCALE("Tanimsiz Islem")
			return		
		
		if (self.Environment == "64"):
			symlink_targets_file.append(["auth\\DevIL.dll", "..\\..\\Server\\share\\bin\\DevIL_x64.dll"])
			symlink_targets_file.append(["game99\\DevIL.dll", "..\\..\\Server\\share\\bin\\DevIL_x64.dll"])
			symlink_targets_file.append(["kanal\\DevIL.dll", "..\\..\\Server\\share\\bin\\DevIL_x64.dll"])
		elif (self.Environment == "86"):
			symlink_targets_file.append(["auth\\DevIL.dll", "..\\..\\Server\\share\\bin\\DevIL_x86.dll"])
			symlink_targets_file.append(["game99\\DevIL.dll", "..\\..\\Server\\share\\bin\\DevIL_x86.dll"])
			symlink_targets_file.append(["kanal\\DevIL.dll", "..\\..\\Server\\share\\bin\\DevIL_x86.dll"])
		else:
			print LOCALE("Tanimsiz Islem")
			return
			
		for i in xrange(len(symlink_targets_dir)):
			os.system("mklink /D/J " + symlink_targets_dir[i][0] + " " + symlink_targets_dir[i][1])
			self.sys_log("%s -> %s %s" % (str(symlink_targets_dir[i][1]), symlink_targets_dir[i][0], LOCALE("dizininde symlink olusturuldu")))

		for i in xrange(len(symlink_targets_file)):
			os.system("mklink /H " + symlink_targets_file[i][0] + " " + symlink_targets_file[i][1])
			self.sys_log("%s -> %s %s" % (str(symlink_targets_file[i][1]), symlink_targets_file[i][0], LOCALE("dosyasinda symlink olusturuldu")))
			
		print LOCALE("Symlink kurulum tamamlandi")
	
	def Yedekal(self):
		print LOCALE("Yedek alma islemi baslatildi")
		
		rarExePath = os.getcwd() + "\\_extern\\rar.exe";
		os.chdir(self.dizin + "\\_backup")
		BACKUP(rarExePath, self.dizin)
		self.sys_log(LOCALE("yedek alindi"))
		
		print LOCALE("Yedek alma islemi tamamlandi")
		
	def Yedekkur(self):
		listdizin = self.dizin+"\\_backup\\"
		r = os.listdir(listdizin)
		for k in r: print k
		sec = raw_input(LOCALE("Lutfen yedek Secin")+"\n")
		if os.path.isfile(self.dizin+"\\_backup\\"+sec):
			print LOCALE("Dosyalari eskileri ile degistirmek icin: Y\nDosyalarin Tumunu Degistirmek Icin: A\nGeri Donmek Icin: Q")
			time.sleep(1)
			os.system(self.dizin + '_extern\\UnRaR.exe'+" x "+self.dizin+"\\_backup\\"+sec+" "+self.dizin)
		
	def usage(self):
		if locale.getdefaultlocale()[0] == "tr_TR":
			print """
Kullanim:
	Oyunu acmak icin: oyun
	Loglari Temizlemek icin: temizle
	Kurulumu kaldirmak icin: kaldir
	Kurulum yapmak icin: kur
	Yedek almak icin: yedekal
	Yedek kurmak icin: yedekkur
	"""
		else:
			print """
Usage:
	To open the game: start
	Clear for the logs: clear
	Remove for installation: remove
	For installation: install
	To make a backup: backup
	For install backup: installbackup
	"""

if __name__ == "__main__":
	M2FilesHelper()
