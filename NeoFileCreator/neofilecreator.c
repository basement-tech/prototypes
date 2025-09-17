#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define NUM_ARGS 14  //
#define MIN_ARGS 0  //
#define DELIM_START '<'
#define DELIM_STOP  '>'

#define MAX_HDR_SIZE 1024

#define PONGA_START_ANGLE -45     // start/reset angle
#define POINTA_SERVO_MASK 0x0001  // which servo to move

static const char header_json[] =
R"==(
{"strategy" : "bitwise", "__comment" : "first line is skipped after noting strategy"}
{
    "label" : "USER-5",
    "strategy" : "bitwise",
    "bonus" : { "depth" : "2", "pixel_cnt" : 64, "brightness" : {"r": 64,  "g": 64, "b": 64, "w": 0}},
    "pixelcnt" : 32
}
)==";

char header[MAX_HDR_SIZE] = {0};
char preamble[256] = {0};
#define BITS_PER_BITMAP 32  // how wide in bits is ach color bitmap entry
typedef struct __attribute__((packed)) {
    int8_t o;  // offset
    uint32_t r, g, b, w;  // color bitmap
    int32_t d;   // delay
    uint16_t s;  // servo bitmap
    int32_t a;   // servo angle
} seq_bin_t;

//
// BEWARE OF STRUCTURE PACKING !!!
//                 Offset    R           G           B           W        T
seq_bin_t data[] = {
                        { 0, 0xA0A0A0A0, 0x00000000, 0x00000000, 0x00000000, 1000, 0x0001, 45},
                        { 1, 0xA0A0A0A0, 0x00000000, 0x00000000, 0x00000000, 1000, 0, 0},
                        { 0, 0xA0A0A0A0, 0x00000000, 0x00000000, 0x00000000, 1000, 0x0004, 45},
                        { 1, 0xA0A0A0A0, 0x00000000, 0x00000000, 0x00000000, 1000, 0, 0},
                        { 0, 0x50505050, 0x00000000, 0x00000000, 0x00000000, 1000, 0x0005, 0},
                        { 1, 0x50505050, 0x00000000, 0x00000000, 0x00000000, 1000, 0, 0},
                        { 0, 0x00000000, 0xA0A0A0A0, 0x00000000, 0x00000000, 1000, 0x0001, -45},
                        { 1, 0x00000000, 0xA0A0A0A0, 0x00000000, 0x00000000, 1000, 0, 0},
                        { 0, 0x00000000, 0xA0A0A0A0, 0x00000000, 0x00000000, 1000, 0x0004, -45},
                        { 1, 0x00000000, 0xA0A0A0A0, 0x00000000, 0x00000000, 1000, 0, 0},
                        { 0, 0x00000000, 0x50505050, 0x00000000, 0x00000000, 1000, 0x0005, 0},
                        { 1, 0x00000000, 0x50505050, 0x00000000, 0x00000000, 1000, 0, 0},
                        { 0, 0x00000000, 0x00000000, 0xA0A0A0A0, 0x00000000, 1000, 0x0001, 10},
                        { 1, 0x00000000, 0x00000000, 0xA0A0A0A0, 0x00000000, 1000, 0, 0},
                        { 0, 0x00000000, 0x00000000, 0x50505050, 0x00000000, 1000, 0x0001, 0},
                        { 1, 0x00000000, 0x00000000, 0x50505050, 0x00000000, 1000, 0, 0},
                        { 0, 0x01010101, 0x00000000, 0x00000000, 0x00000000, -1, 0, 0},
                        { 1, 0x01010101, 0x00000000, 0x00000000, 0x00000000, -1, 0, 0}
                    };

void main(int argc, char **argv)  {
    char filename[128] = "neo_user_6.bseq";
    char label[32] = "USER-6";
    char strategy[32] = "bbitwise";
    char fileopt[32] = "default";
    char bonus[128] = "{ \"depth\" : 2, \"pixel_cnt\" : 64, \"brightness\" : {\"r\": 64,  \"g\": 64, \"b\": 64, \"w\": 0}}";
    char pixelcnt[32] = "64";
    int32_t interval = 1000;  // interval between points
    uint8_t r = 64;
    uint8_t g = 64;
    uint8_t b = 64;
    uint8_t w = 0;
    bool jhdr = true;  // write the json header or not

    int ipixelcnt = 64;
    int idepth = 2;  // depth as integer
    int jlen = 0; // length of the json to follow
    int32_t cbal;  // character balance when creating character strings

    FILE *fp;
    int num = 0;
    int err = 0;
    char arg = '\0';

    if(argc < (MIN_ARGS+1))
        fprintf(stderr, "Error: too few args; filename required\n");
    else if(argc > (NUM_ARGS+1))
        fprintf(stderr, "Error: Too many args ... more than %d\n", NUM_ARGS);
    else  {
        bool needhyphen = true;
        argv++;--argc;
        while((argc-- > 0) && (err >= 0))  {
            if(needhyphen == true)  {
                if((*argv)[0] != '-')  {
                    fprintf(stderr, "Error: malformed arguments\n");
                    err = -1;
                }
                else  {
                    switch((*argv)[1])  {
                        case 'f':  // filename
                        case 'p':  // pixelcount
                        case 'o':  // file contents option
                        case 't':  // time interval
                        case 'c':  // color r g b w
                            arg = (*argv)[1];
                            needhyphen = false;
                            break;

                        case 'x':  // no json header (for debugging)
                            jhdr = false;  // no argument
                            needhyphen = true;
                            break;

                        default:
                            fprintf(stderr, "Error: unknown argument: %s\n", *argv);
                            err = -1;
                            break;
                    }
                    argv++;
                }
            }
            else  {
                switch(arg)  {
                    case 'f':  // filename
                        strncpy(filename, *argv, sizeof(filename));
                        break;

                    case 'p':  // pixelcount
                        strncpy(pixelcnt, *argv, sizeof(pixelcnt));
                        break;

                    case 'o':  // file content option
                        strncpy(fileopt, *argv, sizeof(fileopt));
                        break;

                    case 't':  // time interval
                        interval = atoi(*argv);
                        break;

                    case 'c':  // time interval
                        r = atoi(*argv++);
                        argc--;
                        g = atoi(*argv++);
                        argc--;
                        b = atoi(*argv++);
                        argc--;
                        w = atoi(*argv);
                        break;

                    default:
                        fprintf(stderr, "Error: unknown argument value: %s\n", *argv);
                        err = -1;
                        break;
                }
                argv++;
                needhyphen = true;
            }
        }
        printf("Using:\n");
        printf("   filename :  \"%s\"\n", filename);
        printf("   pixelcount: \"%s\"\n", pixelcnt);
        printf("   fileopt:    \"%s\"\n", fileopt);
        printf("   color :      %u %u %u %u\n", r, g, b, w);
        printf("   interval:    %d\n", interval);

        if(err >= 0)  {
            if((fp = fopen(filename, "wb")) == NULL)
                fprintf(stderr, "Error: can't open file %s\n", filename);
            else
            {
                ipixelcnt = atoi(pixelcnt);
                idepth = ipixelcnt / BITS_PER_BITMAP;  // remainder truncated
                if((ipixelcnt % BITS_PER_BITMAP) > 0)  // leftovers require a full depth added
                    idepth++;
                printf("   depth(calc): %d\n", idepth);
                printf("\n");

                /*
                 * create the bonus string based on parameters
                 */
                cbal = sizeof(bonus);
                snprintf(bonus, cbal, "{ \"depth\" : ");
                snprintf(bonus+strlen(bonus), (cbal-strlen(bonus)), "%2d, ", idepth);
                snprintf(bonus+strlen(bonus), (cbal-strlen(bonus)), "\"pixel_cnt\" : ");
                snprintf(bonus+strlen(bonus), (cbal-strlen(bonus)), "%3d, ", ipixelcnt);
                snprintf(bonus+strlen(bonus), (cbal-strlen(bonus)), "\"brightness\" : ");
                snprintf(bonus+strlen(bonus), (cbal-strlen(bonus)), "{\"r\": %3u,  \"g\": %3u, \"b\": %3u, \"w\": %3u}}", r, g, b, w);
                printf("generated bonus string: %s\n", bonus);
                
                /*
                 * write the part that is common to all file content options
                 */
                cbal = sizeof(header);  // balance is full buffer at this point

                snprintf(header, cbal, "{\n");
                snprintf(header+strlen(header), (cbal-strlen(header)), "  \"label\" : \"%s\",\n", label);
                snprintf(header+strlen(header), (cbal-strlen(header)), "  \"strategy\" : \"%s\",\n", strategy);
                snprintf(header+strlen(header), (cbal-strlen(header)), "  \"bonus\" : %s,\n", bonus);
                snprintf(header+strlen(header), (cbal-strlen(header)), "  \"__comment\" : \"back and forth\"\n");
                snprintf(header+strlen(header), (cbal-strlen(header)), "}\n");

                uint32_t hdr_len = strlen(header);
                printf("Header as buffer:\n%s\n", header);
                printf("strlen(header) = %d\n", strlen(header));

                snprintf(preamble, cbal, "{\"filetype\" : \"BIN_BW\", \"jsonlen\" : %4u, \"__comment\" : \"points as binary\"}\n", hdr_len);

                if(jhdr == true)  {
                    fwrite(preamble, sizeof(char), strlen(preamble), fp);

                    num = fwrite(header, sizeof(char), strlen(header), fp);
                    printf("%d header characters written\n", num);
                }
                else  {
                    printf("CAUTION: No json header written .. file not runnable\n");
                    printf("  ... use od -w27 -b <filename> to display data\n");
                }

                /*
                 * write the binary part that is unique to each file contents option
                 */

                /*
                 * "default"
                 * the original test data
                 * NOTE: relies on canned data above and initialized values
                 */
                if(strncmp("default", fileopt, strlen(fileopt)) == 0)  {

                    printf("sizeof() each binary data structure = %d\n", sizeof(seq_bin_t));
                    num = fwrite(data, sizeof(uint8_t), sizeof(data), fp);
                    printf("%d binary bytes written\n", num);
                }

                /*
                 * single pixel moving back and forth
                 * all colors equal at this point
                 */
                else if(strncmp("pong", fileopt, strlen(fileopt)) == 0)  {
                    uint32_t mask = 0x01;  // which pixels are on
                    int depth = 0;  // number of 32 bit things required to cover strand
                    int actd = -1;  // depth level that is being lit
                    int point = 0;  // point counter
                    seq_bin_t bindata;  // holds one point depth

                    /*
                     * in this case, the number of pixels is a sarogate 
                     * for the number of points that we'll have in the sequence
                     * to completely traverse the strand
                     */
                    for(point = 0; point < ipixelcnt; point++)  {
                        // first point in the next depth level
                        if((point % (BITS_PER_BITMAP-1)) == 0)  {
                            actd++;
                            mask = 0x01;
                        }
                        for(depth = 0; depth < idepth; depth++)  {
                            bindata.o = depth;
                            bindata.s = 0;  // no servo movement
                            bindata.a = 0;  // no servo movement
                            bindata.d = interval;  // 1000 mS by default

                            if(depth == actd)  {
                                bindata.r = mask;
                                bindata.g = mask;
                                bindata.b = mask;
                                bindata.w = mask;
                            }
                            else  {
                                bindata.r = 0;
                                bindata.g = 0;
                                bindata.b = 0;
                                bindata.w = 0;
                            }
                            fwrite(&bindata, sizeof(uint8_t), sizeof(bindata), fp);
                        }
                        mask = mask << 1;
                    }
                    /*
                     * write the termination point
                     */
                    for(depth = 0; depth < idepth; depth++)  {
                        bindata.o = depth;
                        bindata.s = 0;  // no servo movement
                        bindata.a = 0;  // no servo movement
                        bindata.d = -1;  // termination value

                        bindata.r = 0;
                        bindata.g = 0;
                        bindata.b = 0;
                        bindata.w = 0;

                        fwrite(&bindata, sizeof(uint8_t), sizeof(bindata), fp);
                    }
                }
                /*
                 * single pixel moving back and forth
                 * all colors equal at this point
                 * sweep a single servo as the active pixel moves
                 */
                else if(strncmp("ponga", fileopt, strlen(fileopt)) == 0)  {
                    uint32_t mask = 0x01;  // which pixels are on
                    int depth = 0;  // number of 32 bit things required to cover strand
                    int actd = -1;  // depth level that is being lit
                    int point = 0;  // point counter

                    seq_bin_t bindata;  // holds one point depth

                    int32_t angle = 0;  // angle to write
                    int32_t ainc = 0;   // how much to incremet angle per point

                    angle = -45;  // start servo angle here
                    ainc = 90 / ipixelcnt;  // move this much per point (intentionally drop remainder)

                    /*
                     * in this case, the number of pixels is a sarogate 
                     * for the number of points that we'll have in the sequence
                     * to completely traverse the strand
                     */
                    for(point = 0; point < ipixelcnt; point++)  {
                        // first point in the next depth level
                        if((point % (BITS_PER_BITMAP-1)) == 0)  {
                            actd++;
                            mask = 0x01;
                        }
                        for(depth = 0; depth < idepth; depth++)  {
                            bindata.o = depth;
                            bindata.s = 0;  // no servo movement
                            bindata.a = 0;  // no servo movement
                            bindata.d = interval;  // 1000 mS by default

                            /*
                             * set the servo mask and angle at depth 0 only
                             */
                            if(depth == 0)  {  // write the angle only on depth 0
                                if(point == 0)
                                    angle = PONGA_START_ANGLE;
                                else
                                    angle += ainc;
                                bindata.a = angle;
                                bindata.s = POINTA_SERVO_MASK;  // hardcoded to first servo channel
                            }

                            /*
                             * set the bitmasks depending on where in the depth we ar
                             */
                            if(depth == actd)  {
                                bindata.r = mask;
                                bindata.g = mask;
                                bindata.b = mask;
                                bindata.w = mask;
                            }
                            else  {
                                bindata.r = 0;
                                bindata.g = 0;
                                bindata.b = 0;
                                bindata.w = 0;
                            }
                            fwrite(&bindata, sizeof(uint8_t), sizeof(bindata), fp);
                        }
                        mask = mask << 1;
                    }
                    /*
                     * write the termination point
                     */
                    for(depth = 0; depth < idepth; depth++)  {
                        bindata.o = depth;
                        bindata.s = 0;  // no servo movement
                        bindata.a = 0;  // no servo movement
                        bindata.d = -1;  // termination value

                        bindata.r = 0;
                        bindata.g = 0;
                        bindata.b = 0;
                        bindata.w = 0;

                        fwrite(&bindata, sizeof(uint8_t), sizeof(bindata), fp);
                    }
                }
                else
                    fprintf(stderr, "Error: bad file contents option (\"%s\")", fileopt);

                fclose(fp);
            }
        }
        else
            fprintf(stderr, "USAGE: neofilecreator.exe -p <numpixels> -o <defaule, pong> -f <filename> -t <mS> -c <r g b w> -x\n");
    }
}