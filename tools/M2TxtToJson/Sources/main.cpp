#include <windows.h>
#include <iostream>
#include "Parser.h"

int MainRoutine(int argc, char *argv[])
{
    printf("TxtToJson Started!\n");

    if (argc < 4)
    {
        printf("Usage: %s <job_type> <file_type> <input_file> [<output_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }
    auto jobtype = std::atoi(argv[1]);
    auto filetype = std::atoi(argv[2]);

    switch (jobtype)
    {
        case JOB_TYPE_PARSE_TXT:
        {
            auto inputfile = argv[3];
            if (NParser::ParseTxt(filetype, inputfile, "", false) == false)
                return EXIT_FAILURE;;
        } break;

        case JOB_TYPE_PARSE_JSON:
        {
            auto inputfile = argv[3];
            if (NParser::ParseJson(filetype, inputfile) == false)
                return EXIT_FAILURE;;
        } break;

        case JOB_TYPE_TXT_TO_JSON:
        {
            auto inputfile = argv[3];
            auto outputfile = argv[4];
            if (NParser::ParseTxt(filetype, inputfile, outputfile, true) == false)
                return EXIT_FAILURE;
        } break;

        default:
        {
            printf("Unknown job type: %d\n", jobtype);
            return EXIT_FAILURE;
        } break;
    }

    return EXIT_SUCCESS;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow); 
    
    AllocConsole();

    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);

	auto ret = MainRoutine(__argc, __argv);

//  std::system("PAUSE");

    if (fp)
        fclose(fp);
    FreeConsole();

    return ret;
}

