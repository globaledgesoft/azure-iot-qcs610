// This file consist of header file for the 'Developing video record/Playback using GStreamer on TurboX-C610' project //
 
#include <gst/gst.h>

#define WIDTH_4K  3840            // 4K RESOLUTION WIDTH
#define HEIGHT_4K 2160            // 4K RESOLUTION HEIGHT
#define WIDTH_1K  1920            // 1K RESOLUTION WIDTH
#define HEIGHT_1K 1080            // 1K RESOLUTION HEIGHT 
#define PORT 8900                 // PORT for tcp
#define FPS 30                    // Frame per sec 


typedef struct __app_config{
   int width;
   int height;
   char mux[10];
   char rec_choice[10];
   char sink_ops[20];
   char dst[256];
   char name[20];
} APP_CONFIG;


void set_config(APP_CONFIG *config, int width,int height, char * mux, char *rec_choice, char* sink_ops, char* dst, char * name )
{
        // function used to set the configuration parameters//    
    config->width = width;
	config->height = height;
	strcpy(config->mux, mux);
	strcpy(config->rec_choice, rec_choice);
	strcpy(config->sink_ops, sink_ops);
	strcpy(config->dst, dst);
    strcpy(config->name, name);
}
