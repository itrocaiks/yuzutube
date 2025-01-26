#ifndef NCURSES_FUNCS_H
#define NCURSES_FUNCS_H

/*------------------------------------------------------*/
/*--------now this IS the most sinful library-----------*/
/*------------------------------------------------------*/

WINDOW** create_wins(int count, int width) {
    WINDOW** w_arr = (WINDOW**)malloc(count * sizeof(WINDOW*));
    size_t pos = 3;

    for (int i = 0; i < count; ++i) {
        w_arr[i] = newwin(5, width - 2, pos, 1);
        pos += 5;
    }

    return w_arr;
}

WINDOW** create_der_wins2(WINDOW* p_win) {
    WINDOW** w_arr = (WINDOW**)malloc(2 * sizeof(WINDOW*));

    if (shrinked) {
    	w_arr[0] = derwin(p_win, 3, 9, 1, 3);
    	w_arr[1] = derwin(p_win, 3, 9, 1, 13);
    } else {
    	w_arr[0] = derwin(p_win, 3, 18, 1, 3);
    	w_arr[1] = derwin(p_win, 3, 18, 1, 22);
    }
    

    return w_arr;
}

void draw_wins(WINDOW** w_arr, int count, int dif, int width, cJSON*** json_arr) {

    for (int i = 0; i < count; ++i) {
        if (strcmp(json_arr[i+dif][7]->valuestring, "WAIT") != 0) {
            char out_str[strlen(json_arr[i + dif][1]->valuestring)];
            strcpy(out_str, json_arr[i + dif][1]->valuestring);
            if (utf8_strlen(out_str) > (width - 8)) {
                utf8_at(out_str, width - 8)[0] = '\0';
            }
            mvwprintw(w_arr[i], 1, 2, "%d. %s", (i+1+dif), (char*)out_str);

            size_t start2 = 10 - strlen(json_arr[i + dif][3]->valuestring);
            mvwprintw(w_arr[i], 2, start2, "%s | %s", json_arr[i + dif][3]->valuestring, json_arr[i + dif][2]->valuestring);  
        }

        
    }
}

void refresh_wins(WINDOW** w_arr, int count) {

    for (int i = 0; i < count; ++i) {
        wrefresh(w_arr[i]);
    }
}

void erase_wins(WINDOW** w_arr, int count) {

    for (int i = 0; i < count; ++i) {
        werase(w_arr[i]);
    }
}

void delete_wins(WINDOW** w_arr, int count) {

    for (int i = 0; i < count; ++i) {
        delwin(w_arr[i]);
    }
}

void print_search(WINDOW* input_win, int visible_width, int pos, int offset, wchar_t* input) {
	if (shrinked) {
		mvprintw(0, 2, "─────────────");
		mvprintw(0, 2, "Search");
	} else {
		mvprintw(0, 2, "──────────────────────────────────────────────");
    	mvprintw(0, 2, "Search (press Enter to search, Ctrl+X to quit)");
	}
    refresh();

    mvwprintw(input_win, 0, 1, ">");      
    wclrtoeol(input_win);          

    //mvwprintw(input_win, 0, 3, "%.*ls", visible_width, input + offset);
    mvwprintw(input_win, 0, 3, "%ls", input + offset);
    //mvwprintw(input_win, 0, 3, "%ls", input); 
    wmove(input_win, 0, 3 + pos - offset);
    wrefresh(input_win);           
}

void get_sizes(int* height, int* width) {
	curs_set(0);
	getmaxyx(stdscr, *height, *width);

	int h = *height;
	int w = *width;

	if (!is_first_exec) {
		for (int i = 0; i < 3; ++i) {
			usleep(150000);
			char warn[30];
			snprintf(warn, sizeof(warn), "New size: %dx%d", h, w);
	        mvprintw(h / 2, (w / 2) - strlen(warn) / 2, "%s", warn);
	        refresh();
	        usleep(150000);
	        erase();
	        refresh();
		}
		resize_flag = 0;
	}

    if (w < 45 || h < 5) {
        endwin();
        printf("Terminal size should be at least 10x60\n");
        kill(getppid(), SIGTERM);
        exit(1);
    }

    if ((!allow_shrink && w < 60) || h < 10) {
        char* warn1 = "Terminal size should be";
        char* warn2 = "at least 10x60";
        mvprintw(h / 2, (w / 2) - 23 / 2, "%s", warn1);
        mvprintw(h / 2 + 1, (w / 2) - 14 / 2, "%s", warn2);
        refresh();
        while (1) {
        	usleep(100000);
        }  
    }

    if (allow_shrink && w < 60) {
    	shrinked = 1;
    }

    curs_set(1);
}

void draw_bottom(int height, int width, int is_sel) {
	if (!is_sel) {
		mvprintw(height - 1, 1, "─");
		mvprintw(height - 1, width - 2, "─");
	}
	for (int i = 2; i < width - 3; ++i) {
		mvprintw(height - 1, i, "─");
	}
}

void draw_main() {
	initscr();            
    start_color();
    cbreak();             
    noecho();             
    keypad(stdscr, TRUE); 

	int height, width;
    get_sizes(&height, &width);

	WINDOW *input_win = newwin(2, width - 1, 1, 0);           
    keypad(input_win, TRUE);
    wrefresh(input_win);

    box(stdscr, 0, 0);
    

    wchar_t input[MAX_INPUT_Q]; 
    int pos = 0, offset = 0;  
    int ch = 0;
    wint_t wch = 0;          
    int visible_width = width - 5;

    int srch_err = 0;
    int negative_shift = 0;

    while (1) {
    	if (srch_err) {
    		ch = wget_wch(input_win, &wch);
    		print_search(input_win, visible_width, pos, offset, input);
    		srch_err = 0;
    		curs_set(1);
    	} else {
    		print_search(input_win, visible_width, pos, offset, input);
    		ch = wget_wch(input_win, &wch);
    	}
        

        
        if (wch == 10) { 
            curs_set(0);
            nodelay(input_win, TRUE);

            cJSON* root = NULL;

            pos = wcslen(input) - 1;

            wchar_t cp_input[wcslen(input)];
            wcscpy(cp_input, input);

            cJSON*** json_parsed = get_json_parsed(cp_input, &root);

            nodelay(input_win, FALSE);
            

            if (json_parsed == NULL) {
            	if (shrinked) {
            		mvprintw(0, 2, "─────────────");
                	mvprintw(0, 2, "Srch err");
            	} else {
            		mvprintw(0, 2, "──────────────────────────────────────────────");
                	mvprintw(0, 2, "Searching error, try again");
            	}
                
                refresh();
                srch_err = 1;
                continue;
            }

            if (shrinked) {
            	mvprintw(0, 2, "─────────────");
            	mvprintw(0, 2, "‎Found: %d", RECEIVED);
            } else {
            	mvprintw(0, 2, "──────────────────────────────────────────────");
            	mvprintw(0, 2, "‎Found results: %d (press 'q' to input query)", RECEIVED);
            }
            
            refresh();
        
            int max_wins = (height - 3) / 5;
            if (max_wins > RECEIVED) {
                max_wins = RECEIVED;
            }

            WINDOW** w_arr = create_wins(max_wins, width);

            draw_wins(w_arr, max_wins, 0, width, json_parsed);
            box(w_arr[0], 0, 0);
            refresh_wins(w_arr, max_wins);

            draw_bottom(height, width, 0);
            refresh();


            int selected = 0;
            int dif = 0;
            int is_dwnld = 0;
            while (is_dwnld || (ch = wgetch(input_win)) != 113) {

                int is_sel = 0;
                if (!is_dwnld) {
                    switch (ch) {
                        case KEY_UP:
                            if (selected > 0) {
                                selected--;
                            }
                            break;
                        case KEY_DOWN:
                            if (selected < RECEIVED - 1) {
                                selected++;
                            }
                            break;
                        case 10:
                            json_parsed[selected][7]->valuestring = "WAIT";
                            is_sel =1;
                            

                    }
                }

                erase_wins(w_arr, max_wins);

                if (dif > selected - (max_wins - 1)) {
                    if (selected - dif < 0) {
                        dif--;
                    }
                } else if (selected > max_wins - 1) {
                    dif = selected - (max_wins - 1);
                }

                // mvprintw(0, 2, "──────────────────────────────────────────────");
                // mvprintw(0, 2, "Selected: %d (press 'q' to input query)", selected + 1);
                // refresh();
                
                draw_wins(w_arr, max_wins, dif, width, json_parsed);
                if (selected < RECEIVED) {
                    box(w_arr[selected - dif], 0, 0);
                }
                refresh_wins(w_arr, max_wins);

                draw_bottom(height, width, ((selected - dif) == max_wins - 1));
                refresh();

                if (is_dwnld) {
                    char icons[4] = {'|', '/', '-', '\\'};
                    int i_icons = 0;
                    char* format = (strcmp(json_parsed[selected][7]->valuestring, "DWNLD4") == 0) ? "MP4" : "MP3";

                    char command[strlen(CONFIG_PATH) * 2 + 3 + strlen(json_parsed[selected][6]->valuestring) + strlen(DOWNLOAD_DIR) + strlen(json_parsed[selected][1]->valuestring) + 100];

                    snprintf(command, sizeof(command), "python %s/bin/downloader.py -f \"%s\" -c %s/conf/cookies.txt -u \"%s\" -o %s/\"%s\" > /dev/null 2>&1", CONFIG_PATH, format, CONFIG_PATH, json_parsed[selected][6]->valuestring, DOWNLOAD_DIR, json_parsed[selected][1]->valuestring);


                    py_proc = fork();
                    if (py_proc == 0) {
                        setpgid(0,0);
                        system(command);
                        exit(0);
                    }
                    if (shrinked) {
                    	mvwprintw(w_arr[selected - dif], 1, width - 19, "                ");
                    	mvwprintw(w_arr[selected - dif], 2, width - 19, "                ");
                    } else {										 
                    	mvwprintw(w_arr[selected - dif], 1, width - 27, "                        ");
                    	mvwprintw(w_arr[selected - dif], 2, width - 27, "                        ");
                    }
                    

                    char tmp_ch;
                    nodelay(input_win, TRUE);
                    while (1) {
                        tmp_ch = wgetch(input_win);
                        if (tmp_ch == 113) {
                        	if (shrinked) {
                        		mvwprintw(w_arr[selected - dif], 2, width - 19, "                ");
                            	mvwprintw(w_arr[selected - dif], 2, width - 16, "Interrupted");
                        	} else {
                        		mvwprintw(w_arr[selected - dif], 2, width - 27, "                        ");
                            	mvwprintw(w_arr[selected - dif], 2, width - 19, "Interrupted");
                        	}
                            
                            wrefresh(w_arr[selected - dif]);
                            killpg(py_proc, SIGKILL);
                            waitpid(py_proc, NULL, 0);
                            py_proc = -1;
                            break;
                        }
                        
                        int status;
                        pid_t result = waitpid(py_proc, &status, WNOHANG);
                        
                        if (result == -1) {
                        	if (shrinked) {
                        		mvwprintw(w_arr[selected - dif], 2, width - 17, "Err download");
                        	} else {
                        		mvwprintw(w_arr[selected - dif], 2, width - 24, "Error downloading %s", format);
                        	}
                            
                            wrefresh(w_arr[selected - dif]);
                            break;
                        }
                        
                        if (result == 0) {
                        	if (shrinked) {
                        		mvwprintw(w_arr[selected - dif], 2, width - 17, "Downloadin %c", icons[i_icons]);
                        	} else {
                        		mvwprintw(w_arr[selected - dif], 2, width - 22, "Downloading %s %c", format, icons[i_icons]);
                        	}
                            
                            if (i_icons == 3) {
                                i_icons = 0;
                            } else {
                                i_icons++;
                            }
                        } else {
                        	if (shrinked) {
                        		mvwprintw(w_arr[selected - dif], 2, width - 18, "%s downloaded", format);
                        	} else {
                        		mvwprintw(w_arr[selected - dif], 2, width - 22, "Downloaded! (%s)", format);
                        	}
                            
                            wrefresh(w_arr[selected - dif]);
                            py_proc = -1;
                            break;
                        }

                        wrefresh(w_arr[selected - dif]);
                        usleep(200000);
                    }
                    nodelay(input_win, FALSE);
                    json_parsed[selected][7]->valuestring = "none";
                    is_dwnld = 0;
                }

                if (is_sel) {

                    WINDOW** der_wins = create_der_wins2(w_arr[selected-dif]);
                    box(der_wins[0], 0, 0);

                    if (shrinked) {
                    	mvwprintw(der_wins[0], 1, 2, "MP3 ♬ ");
                    	mvwprintw(der_wins[1], 1, 2, "MP4 ⏵︎ ");
                    } else {
                    	mvwprintw(der_wins[0], 1, 2, "Download MP3 ♬ ");
                    	mvwprintw(der_wins[1], 1, 2, "Download MP4 ⏵︎ ");
                    }
                    

                    refresh_wins(der_wins, 2);

                    int sel_der = 0;
                    int ext_loop = 0;
                    while ((ch = wgetch(input_win)) != 113) {
                        
                        switch (ch) {
                            case KEY_LEFT:
                                if (sel_der > 0) {
                                    sel_der--;
                                }
                                break;
                            case KEY_RIGHT:
                                if (sel_der < 1) {
                                    sel_der++;
                                }
                                break;
                            case 10:
                                json_parsed[selected][7]->valuestring = (sel_der) ? "DWNLD4" : "DWNLD3";
                                is_sel = 0;
                                is_dwnld = 1;
                                ext_loop = 1;

                        }
                        if (ext_loop) {
                            break;
                        }

                        erase_wins(der_wins, 2);

                        if (shrinked) {
	                    	mvwprintw(der_wins[0], 1, 2, "MP3 ♬ ");
	                    	mvwprintw(der_wins[1], 1, 2, "MP4 ⏵︎ ");
	                    } else {
	                    	mvwprintw(der_wins[0], 1, 2, "Download MP3 ♬ ");
	                    	mvwprintw(der_wins[1], 1, 2, "Download MP4 ⏵︎ ");
	                    }

                        if (sel_der != -1 && sel_der < 2) {
                            box(der_wins[sel_der], 0, 0);
                        }

                        refresh_wins(der_wins, 2);
                            
                    }
                    
                    erase_wins(der_wins, 2);
                    refresh_wins(der_wins, 2);
                    delete_wins(der_wins, 2);
                    if (ch == 113) {
                        json_parsed[selected][7]->valuestring = "none";
                        is_sel = 0;
                        draw_wins(w_arr, max_wins, dif, width, json_parsed);
                        refresh_wins(w_arr, max_wins);
                    }
                    free(der_wins);
                }

                
            }

            delete_wins(w_arr, max_wins);
            curs_set(1);

            free_json_arr(&json_parsed);
            free(json_parsed); 
            free(w_arr); 
            cJSON_Delete(root);
            
        } else if (wch == KEY_BACKSPACE || wch == 127) { 
            if (pos > 0) {          
                pos--;
                memmove(input + pos, input + pos + 1, wcslen(input) * sizeof(wchar_t));
                //input[pos] = L'\0';
                
                if (pos < offset) {
                    offset--;
                }

                if (pos + 2 > visible_width) {
                    offset--;
                }
            }
        } else if (wch == KEY_LEFT) { 
            if (pos > 0) {
                negative_shift++;          
                pos--;
                
                if (pos < offset) {
                    offset--;
                }

                if (pos + 2 > visible_width) {
                    offset--;
                }
            }
        } else if (wch == KEY_RIGHT) { 
            if (pos > 0 && pos < wcslen(input)) {
                negative_shift--;          
                pos++;
                

                if (pos - offset >= visible_width) {
                    offset++;
                }
                // if (pos > offset) {
                //     offset++;
                // }

                // // if (pos + 2 > visible_width) {
                // //     offset--;
                // // }
            }
        } else if (pos < MAX_INPUT_Q - 1 && wch >= 32) { 
            if (negative_shift) {
                memmove(input + pos + 1, input + pos, (wcslen(input) + 2) * sizeof(wchar_t));
                input[pos++] = wch;
            } else {
                input[pos++] = wch;
                input[pos] = '\0';
            }
            

            if (pos - offset >= visible_width) {
                offset++;
            }
        } else if (wch == 24) {
            break;
        }
    }

    delwin(input_win);  
    endwin();           
    kill(getppid(), SIGTERM);
}

#endif
