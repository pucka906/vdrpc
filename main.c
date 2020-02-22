#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "include/discord_rpc.h"

const char* APPLICATION_ID = "671691503026044938";

const char ext[30][12] = {
	".docker", ".npmrc", ".travis", ".cpp", ".css", ".cs", ".c", ".htm", ".php", ".sh", ".bash", ".hs", ".h", ".java",
	".json", ".js", ".py", ".rb", ".rs", ".go", ".kt", ".sass", ".svg", ".swift", ".yml", ".md", ".log", ".git", ".vim", ".xml"
};

void InitDiscord(){
	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));
	Discord_Initialize(APPLICATION_ID, &handlers, 1, NULL);
}

void UpdatePresence(char *details, char *state, char *largeImageKey, char *largeImageText, time_t timestamp){
	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));

	discordPresence.details = details;
	discordPresence.state = state;

	int len = sizeof(ext) / sizeof(ext[0]);
	int i = 0;

	while(i < len)
	{
		if(strstr(largeImageKey, ext[i])){
			break;
		}
		i++;
	}

	char key[32];

	if(i == len){
		discordPresence.largeImageKey = "default";
	} else {
		strcpy(key, ext[i]);
		memmove(key, key + 1, strlen(key));
		discordPresence.largeImageKey = key;
	}

	discordPresence.largeImageText = largeImageText;
	discordPresence.startTimestamp = timestamp;

	Discord_UpdatePresence(&discordPresence);
}

void MainLoop(){
	time_t start = time(0);

	while(!sleep(5)){
		FILE *f = fopen("/tmp/vdrpc", "r");
		if(!f) return;

		char buffer[3][256];

		if(!fgets(buffer[0], sizeof(buffer[0]), f)) continue;   // get details
		fgets(buffer[1], sizeof(buffer[1]), f);                 // get state
		fgets(buffer[2], sizeof(buffer[2]), f);                 // get size

		char *details = buffer[0];
		char *state = buffer[1];
		char largeImageKey[32];
		char largeImageText[128] = "text";

		if(!strcmp(details, "Editing \n")){
			details = "Editing an unnamed file.";
		}

		if(strstr(buffer[2], "Size:") != NULL){
			state = strcat(buffer[2], " bytes");
		}

		int i, j, b = 0;

		for(i = j = 0; details[i] != '\0'; i++){
			if(details[i] == '.'){	// if [i - 1] -> .c = c
				b = 1;
			}
			if(b){
				largeImageKey[j++] = details[i];
			}
		}

		largeImageKey[strlen(largeImageKey + 1)] = '\0';	// add zero byte

		UpdatePresence(details, state, largeImageKey, largeImageText, start);

		fclose(f);
	}
}

int main(){
	InitDiscord();
	MainLoop();
	Discord_Shutdown();
	return 0;
}
