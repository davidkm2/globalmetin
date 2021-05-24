#include "../include/FileMonitor.h"

#include <cstdlib>
#include <chrono>
#include <filesystem>

#if (TARGET_ARCH == 64)
	#define GDB_COMMANDS_FILE "x64_dbg_commands.txt"
#elif (TARGET_ARCH == 32)
	#define GDB_COMMANDS_FILE "x86_dbg_commands.txt"
#else
	#error "Unknown architecture"
#endif

int32_t main(int32_t argc, char *argv[])
{
#ifdef __FreeBSD__
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <target_file>\n", argv[0]);
		return 1;
	}
	auto target_file = argv[1];
	fprintf(stderr, "Corewatcher started target: '%s'\n", target_file);

	auto _file_monitor = std::make_unique<CFileMonitor>();

	auto ret = _file_monitor->AppendFile(
		target_file,
		[](const std::string &file, int32_t flag) -> void
		{
			for (const auto & entry : std::filesystem::directory_iterator(file))
			{
				if (entry.path().extension() == ".core")
				{
					fprintf(stdout, "Core file found: '%s'\n", entry.path().c_str());

					auto basefilesize = std::filesystem::file_size(entry.path());
					auto basetime = std::chrono::high_resolution_clock::now();
					while (true)
					{
						usleep(500);

						if (basefilesize == std::filesystem::file_size(entry.path()))
							break;
						basefilesize = std::filesystem::file_size(entry.path());

						if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - basetime).count() > 10000)
						{
							fprintf(stderr, "file waiting timeout!");
							return;
						}
					}

					auto lockedname = std::string(entry.path().c_str()) + ".locked";
					std::filesystem::rename(entry.path(), std::filesystem::path(lockedname));

					auto st = std::time(nullptr);
    				auto secs = static_cast<std::chrono::seconds>(st).count();
    				auto timestamp = static_cast<uint64_t>(secs);
    				auto outfile = "core_content_" + std::to_string(timestamp) + ".txt";

					if (std::filesystem::exists(GDB_COMMANDS_FILE) == false)
					{
						fprintf(stderr, "gdb command file not found!\n");
						return;
					}

					auto fmtcommand = "gdb --command " + std::string(GDB_COMMANDS_FILE) + " > " + outfile;

					std::system(fmtcommand.c_str());

					fprintf(stdout, "Output writed to: '%s'\n", outfile.c_str());

					std::filesystem::remove(lockedname);
				}
			}
        }
    );

    if (ret == false)
    {
    	fprintf(stderr, "Appendfile fail!\n");
    	return 1;
    }

	while (true)
	{
		_file_monitor->OnTick();
		usleep(5);
	}
#endif

	return 0;
}

