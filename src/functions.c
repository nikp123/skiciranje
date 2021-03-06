#include "shared.h"

int loadConfig()
{
	/** todo later
#ifdef __linux__
char *configFile = "~/.config/skiciranje.conf";
#endif**/
	return 0;
}

int loadLanguage(char *language) {
	char *tempString = (char*) malloc(1000*sizeof(char));
	if(tempString == NULL) {
		SDL_Log("MEMORY ERROR!\n");
		return 1;
	}


	// todo implement ability to change language
	char languageFile[100] = {0};
	strcat(languageFile, "assets/lang/");
	strcat(languageFile, language);

	// load the file into memory
	FILE *file = fopen(languageFile, "rb");
	if(file == NULL) {
		SDL_Log("Failed to open: %s\n", languageFile);
		return 1;
	}

	textLines = 0;              // set line counter to 0

	// count textlines
	while(fgets(tempString, 1000, file)!=NULL) textLines++;

	// allocate them
	textLine = (char**) malloc(textLines*sizeof(char*));
	if(textLine == NULL) {
		SDL_Log("Memory error!\n");
		return 1;
	}

	// jump back to the beginning
	fseek(file, 0, SEEK_SET);

	// load all the strings in
	for(int i = 0; i < textLines; i++) {
		fgets(tempString, 1000, file);
		textLine[i] = (char *)malloc(sizeof(char)*strlen(tempString)+1);
		if(textLine[i] == NULL) {
			SDL_Log("Memory error!\n");
			return 1;
		}
		strcpy(textLine[i], tempString);

		// NULL-termintate strings to avoid fucking memory corruptions
        if(textLine[i][strlen(tempString)-1] == 0x0a) 
            textLine[i][strlen(tempString)-1] = '\0';
        else textLine[i][strlen(tempString)] = '\0';
	}

	free(tempString);
	fclose(file);
	return 0;
}

SDL_Color translate_color(Uint32 int_color)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	SDL_Color color={(int_color & 0x00ff0000)/0x10000,(int_color & 0x0000ff00)/0x100,(int_color & 0x000000ff),0};
#else
	SDL_Color color={(int_color & 0x000000ff),(int_color & 0x0000ff00)/0x100,(int_color & 0x00ff0000)/0x10000,0};
#endif
	return color;
}
