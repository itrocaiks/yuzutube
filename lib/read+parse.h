#ifndef READ_PARSE_H
#define READ_PARSE_H

/*------------------------------------------------------*/
/*--------------please, close this file-----------------*/
/*----------i commited 279 war crimes here--------------*/
/*------------------------------------------------------*/

cJSON*** read_json(char* json_string, cJSON* root) {
    if (json_string == NULL) {
        return NULL;
    }

    root = cJSON_Parse(json_string);
    if (root == NULL) {
    	endwin();
        printf("Error parsing JSON (this is a temporary error, try again).\n");
        kill(getppid(), SIGTERM);
        exit(1);
    }


    cJSON *videos = cJSON_GetObjectItem(root, "videos");
    if (!cJSON_IsArray(videos)) {
    	endwin();
        printf("Error: 'videos' is not an array (this is a temporary error, try again).\n");
        cJSON_Delete(root);
        kill(getppid(), SIGTERM);
        exit(1);
    }

    RECEIVED = cJSON_GetArraySize(videos);
    if (RECEIVED == 0) {
    	cJSON_Delete(root);
    	return NULL;
    }


    cJSON*** json_arr = (cJSON***)malloc(RECEIVED * sizeof(cJSON**));

    for (int i = 0; i < RECEIVED; ++i) {
	    cJSON *video = cJSON_GetArrayItem(videos, i);
	    if (!cJSON_IsObject(video)) {
	    	endwin();
	        printf("Error: element of an array 'videos' is not an object (this is a temporary error, try again).\n");
	        cJSON_Delete(root);
	        kill(getppid(), SIGTERM);
	        exit(1);    
	    }

	    json_arr[i] = (cJSON**)malloc(8 * sizeof(cJSON*));

	    json_arr[i][0] = cJSON_GetObjectItem(video, "id");
	    json_arr[i][1] = cJSON_GetObjectItem(video, "title");
	    json_arr[i][2] = cJSON_GetObjectItem(video, "channel");
	    json_arr[i][3] = cJSON_GetObjectItem(video, "duration");
	    json_arr[i][4] = cJSON_GetObjectItem(video, "views");
	    json_arr[i][5] = cJSON_GetObjectItem(video, "publish_time");
	    json_arr[i][6] = cJSON_GetObjectItem(video, "url_suffix");

		if (cJSON_AddStringToObject(video, "state", "none") == NULL) {
			kill(getppid(), SIGTERM);
	    	exit(1);
	    }
		json_arr[i][7] = cJSON_GetObjectItem(video, "state");
		
	    //json_arr[i][8] = cJSON_GetObjectItem(video, "thumbnails");
    }
    


    return json_arr;
}

void free_json_arr(cJSON**** json_arr) {
	if (RECEIVED != 0) {
		for (int i = 0; i < RECEIVED; ++i) {
			free((*json_arr)[i]);
		}
	}
}

char* read_file(char* file_name) {
    FILE *file;
    char *buffer = NULL;
    char temp_buffer[128];
    size_t buffer_size = 0;
    size_t chunk_len;

    file = fopen(file_name, "r");
    if (file == NULL) {
       return NULL;
    }

    while (fgets(temp_buffer, sizeof(temp_buffer), file) != NULL) {
        chunk_len = strlen(temp_buffer);

        char *new_buffer = realloc(buffer, buffer_size + chunk_len + 1);
        if (new_buffer == NULL) {
            free(buffer);
            fclose(file);
            return NULL;
        }
        buffer = new_buffer;

        strcpy(buffer + buffer_size, temp_buffer);
        buffer_size += chunk_len;
    }

    if(ferror(file)) { 
		free(buffer);
		fclose(file);
		return NULL;
    }

    fclose(file);

    return buffer; 
}

void parse_input(char** input) {
    // if (utf8_at(*input, 0)[0] == '\"') {
    //     utf8_shift_left(*input, 0);
    // }
    // if (utf8_at(*input, utf8_strlen(*input) - 1)[0] == '\"') {
    //     utf8_at(*input, utf8_strlen(*input) - 1)[0] = '\0';
    // }

    for (int i = 0; i < utf8_strlen(*input); ++i) {
        if (utf8_at(*input, i)[0] == '\\'||
            utf8_at(*input, i)[0] == '\"' ||
            utf8_at(*input, i)[0] == '\'') {
            *input = utf8_remove_char(*input, i);
        i--;
        }
    }
}

wchar_t* remove_wchar(wchar_t* str, int index) {

    // wchar_t str_tmp1[wcslen(str)];
    // wcscpy(str_tmp1, str);
    // str_tmp1[index] = '\0';


    //wchar_t str_tmp2[wcslen(str)];
    //wcscpy(str_tmp2, str);
    //memmove(str_tmp2 + index, str_tmp2 + index + 1, (wcslen(str_tmp2) + 1) * sizeof(wchar_t));

    memmove(str + index, str + index + 1, (wcslen(str) + 1) * sizeof(wchar_t));


    //wcscpy(str, str_tmp2);
    

    //swprintf(str, wcslen(str) + wcslen(str_tmp1) + wcslen(str_tmp2), L"%ls%ls", str_tmp1, str_tmp2);

    return str;

}

void parse_winput(wchar_t* input) {

    for (int i = 0; i < wcslen(input); ++i) {
        if (input[i] == '\\'||
            input[i] == '\"' ||
            input[i] == '\'') {
            input = remove_wchar(input, i);
            i--;
        }
    }
}

cJSON*** get_json_parsed(wchar_t* input, cJSON** root) {
	size_t len = wcslen(input);

    parse_winput(input);

    size_t command_len = strlen(CONFIG_PATH) * 2 + len * sizeof(wchar_t) + 80;
	char py_command[command_len];
	snprintf(py_command, sizeof(py_command), "python %s/bin/query_sender.py -q \"%ls\" -l %d -c %s > /dev/null 2>&1", CONFIG_PATH, input, MAX_LIMIT, CONFIG_PATH);

	py_proc = fork();
	if (py_proc == 0) {
        
		int ret_code = system(py_command);
		if (ret_code == 0) {
			exit(0);
		} else {
			exit(-1);
		}
    	
	}

	char* icons[3] = {".", "..", "..."};
    int i_icons = 0;

	int status;
	while (1) {                  
        pid_t result = waitpid(py_proc, &status, WNOHANG);
        
        if (result == -1) {
        	py_proc = -1;
        	return NULL;
        }
        
        if (result == 0) {
            if (shrinked) {
                mvprintw(0, 2, "─────────────");
                mvprintw(0, 2, "Searching%s", icons[i_icons]);
            } else {
                mvprintw(0, 2, "──────────────────────────────────────────────");
                mvprintw(0, 2, "Searching%s", icons[i_icons]);
            }
        	
            if (i_icons == 2) {
                i_icons = 0;
            } else {
                i_icons++;
            }
        } else if (result == py_proc){
    		if (WEXITSTATUS(status) == 0) {
    			py_proc = -1;
    			break;
    		} else {
    			py_proc = -1;
    			return NULL;
    		}
        	
        } else {
        	py_proc = -1;
        	return NULL;
        }

        refresh();
        usleep(200000);
    }

	char file_name[strlen(CONFIG_PATH) + len * sizeof(wchar_t) + 40];
	snprintf(file_name, sizeof(file_name), "%s/tmp/\'%ls\'.json", CONFIG_PATH, input);

	if (access(file_name, F_OK) != 0) {
	   endwin();
	   printf("Error: searching file %s was not found (check CONFIG_PATH).", file_name);
	   kill(getppid(), SIGTERM);
	   exit(1);
	}


	char *json_string = read_file(file_name);
	cJSON*** json_parsed = read_json(json_string, *root);

	remove(file_name);
	
	return json_parsed;
}

#endif
