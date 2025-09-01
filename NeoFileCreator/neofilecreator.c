#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define NUM_ARGS 2  // filename, strategy
#define MIN_ARGS 1  // filename required
#define DELIM_START '<'
#define DELIM_STOP  '>'

#define MAX_HDR_SIZE 1024

static const char header_json[] =
R"==(
{"strategy" : "bitwise", "__comment" : "first line is skipped after noting strategy"}
{
    "label" : "USER-5",
    "strategy" : "bitwise",
    "bonus" : { "depth" : "2", "pixel_cnt" : 32, "brightness" : {"r": 255,  "g": 0, "b": 0, "w": 0}},
    "pixelcnt" : 32
}
)==";

char header[MAX_HDR_SIZE] = {0};;
char preamble[256] = {0};

//                       R           G           B           W        T
uint32_t data[] = { 0x10101010, 0x00000000, 0x00000000, 0x00000000, 1000,
                    0x01010101, 0x00000000, 0x00000000, 0x00000000, 1000,
                    0x0, 0x0, 0x0, 0x0, -1 };

void main(int argc, char **argv)  {
    char filename[128];
    char label[32] = "USER-5";
    char strategy[32] = "bitwise";
    char bonus[] = "{ \"depth\" : \"2\", \"pixel_cnt\" : 32, \"brightness\" : {\"r\": 255,  \"g\": 0, \"b\": 0, \"w\": 0}}";
    char pixelcnt[32] = "32";
    int jlen = 0; // length of the json to follow

    FILE *fp;
    int num = 0;

    if(argc < (MIN_ARGS+1))
        fprintf(stderr, "Error: too few args; filename required\n");
    else if(argc > (NUM_ARGS+1))
        fprintf(stderr, "Error: Too many args ... more than %d\n", NUM_ARGS);
    else  {
        for(int n = 1; n < argc; n++)  {
            printf("Using filename : %s\n", argv[n]);
            switch(n)  {
                case 0:
                    break;
                case 1:
                    strncpy(filename, argv[n], sizeof(filename));
                    break;
                case 2:
                    strncpy(strategy, argv[n], sizeof(strategy));
                    break;
                default:
                    break;
            }
        }
        if((fp = fopen(filename, "wb")) == NULL)
            fprintf(stderr, "Error: can't open file %s\n", filename);
        else
        {
            uint32_t cbal = sizeof(header);

            snprintf(header, cbal, "{\n");
            snprintf(header+strlen(header), (cbal-=strlen(header)), "  \"label\" : \"%s\",\n", label);
            snprintf(header+strlen(header), (cbal-=strlen(header)), "  \"strategy\" : \"%s\",\n", strategy);
            snprintf(header+strlen(header), (cbal-=strlen(header)), "  \"bonus\" : %s,\n", bonus);
            snprintf(header+strlen(header), (cbal-=strlen(header)), "  \"__comment\" : \"back and forth\"\n");
            snprintf(header+strlen(header), (cbal-=strlen(header)), "}\n");

            uint32_t hdr_len = strlen(header);
            printf("Header as buffer:\n%s\n", header);
            printf("strlen(header) = %d\n", strlen(header));

            snprintf(preamble, cbal, "{\"filetype\" : \"BIN_BW\", \"hdrlen\" : %4u, \"__comment\" : \"points as binary\"}\n", hdr_len);
            fwrite(preamble, sizeof(char), strlen(preamble), fp);

            num = fwrite(header, sizeof(char), strlen(header), fp);
            printf("%d header characters written\n", num);
            num = fwrite(data, sizeof(uint8_t), sizeof(data), fp);
            printf("%d binary bytes written\n", num);
            fclose(fp);
        }
    }
}