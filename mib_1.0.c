#include<ncurses.h>
#include<menu.h>
#include<form.h>
#include<panel.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#define size(a) ( sizeof(a) / sizeof(a[0]) )

MENU *menu;
ITEM **item;
int post;
void swap_fore(int x, char *str) {
	int i = strlen(str);
	while(x <= i) {
		str[x - 1] = str[x];
		x++;
	}
}
void swap_back(int x, char *str) {
	int i = strlen(str);
	while( i >= x) {
		str[i + 1] = str[i];
		i--;
	} 
}
void tokens(char *str, char *delim, char **token) {
	int i = 0;
	token[i] = strtok(str, delim);
	while( token[i] != NULL) {
		i++;
		token[i] = (char*)malloc(31 * sizeof(char));
		token[i] = strtok(NULL, delim);
	}
}

void search_inventory(WINDOW *win, const char *name, char *text, int *location) {
	int i = 0, j = 8, k, l = 1, size = strlen(text);
	char **token, str[280], *delim = ",\n";
	FILE *fp;
	fp = fopen("database.csv", "r");
	if(fp == NULL) {
		mvprintw(LINES - 2, 4, "Database does not exist");
		refresh();
		return;
	}
	if(strcmp(name, "Name") == 0)
		k = 0;
	else if(strcmp(name, "Category") == 0)
		k = 3;
	else if(strcmp(name, "Batch No") == 0)
		k = 4;
	else if(strcmp(name, "Location") == 0)
		k = 5;
	else if(strcmp(name, "Expiry Date") == 0)
		k = 6;
	else if(strcmp(name, "Quantity") == 0)
		k = 7;
		
	wmove(win, 8, 1);
	wclrtobot(win);
	token = (char**)malloc(10 * sizeof(char*));
		
	while(fgets(str, 280, fp) != NULL) {
		i = 0;
		tokens(str, delim, token);
		
		if(strncmp(token[k], text, size) == 0) {
			mvwprintw(win, j, *(location + i), "%d", l);
			while( token[i] != NULL) {
				if(i < 2) 
					mvwprintw(win, j, *(location + i + 1), "%s", token[i]);
				if(i > 2)
					mvwprintw(win, j, *(location + i ), "%s", token[i]);
				i++;
			}
			j++;
			l++;
		}
	}
	free(token);
	fclose(fp);
}
int newitem_process(FORM *form, FIELD **field) {
	int j = 0, i = field_count(form);
	fpos_t position;
	char **token, str[280], *delim = ",\n";
	
	FILE *fp;
	fp = fopen("database.csv", "w+");
	if(fp == NULL) 
		return 0;
		
	token = (char**)malloc(10 * sizeof(char*));
	
	fgetpos(fp, &position);
	while(fgets(str, 280, fp) != NULL) {
		tokens(str, delim, token);
		if( !strcmp(token[0], field_buffer(field[0], 0)) &&  !strcmp(token[1], field_buffer(field[1], 0)) && !strcmp(token[4], field_buffer(field[4], 0)) && !strcmp(token[6], field_buffer(field[6], 0)) ) 
			break;
		fgetpos(fp, &position);
	}

	fsetpos(fp, &position);
	j = 0;
	while(j < (i - 1) ) {
		fprintf(fp, "%30s", field_buffer(field[j], 0));
		if(j <  (i - 2) )
			fprintf(fp, ",");
		else 
			fprintf(fp, "\n");
		j++;
	}
	fclose(fp);
	return 1;
}
void print_items(WINDOW *win, int *location) {
	FILE *fp;
	char str[280], **token;
	char *delim = ",\n";
	int i = 0, j = 8, l =1;
	
	token = (char**)malloc(10 * sizeof(char*));
	
	fp = fopen("database.csv", "r");
	if(fp == NULL) {
		mvprintw(LINES - 2, 4, "Database does not exist");
		refresh();
		return;
	}
	while(fgets(str, 280, fp) != NULL) {
		i = 0;
		tokens(str, delim, token);
		mvwprintw(win, j, *(location + i), "%d", l);
		while( token[i] != NULL) {	
			if(i < 2) 
				mvwprintw(win, j, *(location + i + 1), "%s", token[i]);
			if(i > 2)
				mvwprintw(win, j, *(location + i), "%s", token[i]);
			i++;
		}
		j++;
		l++;
	}
	free(token);
	fclose(fp);
}
int medicine_name(WINDOW *win, FIELD *field, int count) {
	
	char token[31], str[280], **data;
	int i = 0, j;
	FILE *fp;
	fp = fopen("database.csv", "r");
	
	data = (char**) malloc(10 * sizeof(char*));	
	while(fgets(str, 280, fp) != NULL) {
		strcpy(token, strtok(str, ","));
		if(strncmp(token, field_buffer(field, 0), count) == 0) {
			if( i % 10 == 0 )
				data = (char **) realloc(data, ( i + 10 ) * sizeof(char *));
			data[i] = (char *) malloc(31 * sizeof(char));
			strcpy(data[i], token);
			i++;
		}
	}
	if( i == 0)
		return 0;
	if(post == 1) {
		unpost_menu(menu);
		free_menu(menu);
		j = 0;
		while(item[j] != NULL)
			free_item(item[j++]);
		free_item(item[j]);
		free(item);
		post = 0;
	}
	wclear(win);
	item = (ITEM **) malloc( (i + 1) * sizeof(ITEM *) );
	for(j = 0;j < i;j++) {
		item[j] = new_item(data[j], "");
		//free(data[j]);
	}
	free(data);
	item[j] = NULL;
	menu = new_menu(item);
	set_menu_win(menu, win);
	post_menu(menu);
	post = 1;
	wrefresh(win);
	return 1;
}

void newinvoice(WINDOW *win) {
	FIELD **field;
	FORM *form;
	WINDOW *win1, *swin;;
	int i, count = 0, flag = 0;
	char a[31];
	mvwhline(win, 2, 27, ACS_HLINE, COLS - 32);
	mvwvline(win, 1, 26, ACS_VLINE, 1);
	field = (FIELD **)malloc(7 * sizeof(FIELD *));
	field[0] = new_field(1, 30, 0, 1, 0, 0);
	field[1] = new_field(1, 3, 0, 33, 0, 0);
	field[2] = new_field(1, 30, 0, 38, 0, 0);
	field[3] = new_field(1, 30, 2, 1, 0, 0);
	field[4] = new_field(1, 5, 2, 33, 0, 0);
	field[5] = new_field(1, 5, 2, 41, 0, 0);
	field[6] = NULL;
	for(i = 0;i < 5;i++) {
		field_opts_off(field[i], O_AUTOSKIP);
		set_field_back(field[i], COLOR_PAIR(43));
		set_field_fore(field[i], COLOR_PAIR(43));
	} 
	field_opts_off(field[i], O_AUTOSKIP | O_EDIT);
	set_field_back(field[i], COLOR_PAIR(42));
	set_field_fore(field[i], COLOR_PAIR(42));
	set_field_buffer(field[5], 0, " Add ");
	form = new_form(field);
	
	win1 = derwin(win, 3, 70, 4, 1);			
	set_form_win(form, win);
	set_form_sub(form, win1);
	post_form(form);
	mvwprintw(win, 3, 2, "Patient's Name");
	mvwprintw(win, 3, 34, "Age");
	mvwprintw(win, 3, 39, "Doctor's Name");
	mvwprintw(win, 5, 2, "Medicine Name");
	mvwprintw(win, 5, 34, "Qty");
	wrefresh(win);
	
	while(i = wgetch(win)) {
		if(isprint(i) || (i == ' ') || (i == '\t') ) {
			form_driver(form, i);
			form_driver(form, REQ_END_LINE);
			if(current_field(form) == field[3]) {
				count++;
				if(count > 0 && flag == 0) {
					swin = derwin(win, 5, 30, 7, 2);
					flag = 1;
					post = 0;
					wbkgd(swin, COLOR_PAIR(43));
					wrefresh(swin);
				}
				if(medicine_name(swin, field[3], count) == 0)
					goto deletewin;
			}
		}
		else {
			switch(i) {
				case KEY_DOWN:
					if(current_field(form) == field[3] && post == 1) {
						menu_driver(menu, REQ_NEXT_ITEM);
						wrefresh(swin);
					}
					else {
						form_driver(form, REQ_NEXT_FIELD);
						form_driver(form, REQ_END_LINE);
					}
					break;
				case KEY_UP:
					if(current_field(form) == field[3] && post == 1) {
						menu_driver(menu, REQ_PREV_ITEM);
						wrefresh(swin);
					}
					else {
						form_driver(form, REQ_PREV_FIELD);
						form_driver(form, REQ_END_LINE);
					}
					break;
				case KEY_LEFT:
					form_driver(form, REQ_PREV_CHAR);
					break;
				case KEY_RIGHT:
					form_driver(form, REQ_NEXT_CHAR);
					break;
				case 27:
					if(current_field(form) == field[3] && post == 1) {
						set_field_buffer(field[3], 0, "");
						goto deletewin;
					}
					else
						goto end;
						break;
				case KEY_BACKSPACE: case 127:
					form_driver(form, REQ_DEL_PREV);
					form_driver(form, REQ_END_LINE);
					if(current_field(form) == field[3] && count > 0) {
						count--;
						if(count >= 0 && flag == 0) {
							swin = derwin(win, 5, 30, 7, 2);
							flag = 1;
							post = 0;
							wbkgd(swin, COLOR_PAIR(43));
							wrefresh(swin);
						}
						if(medicine_name(swin, field[3], count) == 0)
							goto deletewin;
					}
					break;
				case KEY_DC:
					form_driver(form, REQ_DEL_CHAR);
					form_driver(form, REQ_END_LINE);
					if(current_field(form) == field[3] && count > 0) {
						count--;
						if(count >= 0 && flag == 0) {
							swin = derwin(win, 5, 30, 7, 2);
							flag = 1;
							post = 0;
							wbkgd(swin, COLOR_PAIR(43));
							wrefresh(swin);
						}
						if(medicine_name(swin, field[3], count) == 0)
							goto deletewin;
					}
					break;
				case 10:
					if(current_field(form) == field[3]) {
						if(post == 1) {
							set_field_buffer(field[3], 0, item_name(current_item(menu)) );
							form_driver(form, REQ_NEXT_FIELD);
							goto deletewin;
						}
					}
			}
		}
		if(count == 0 ) {
			deletewin :
			if(flag == 1) {
				if(post == 1) {
					unpost_menu(menu);
					free_menu(menu);
					i = 0;
					while(item[i] != NULL) 
						free_item(item[i++]);
					post = 0;
				}
				wbkgd(swin, COLOR_PAIR(3));
				wclear(swin);
				delwin(swin);
				touchwin(win1);
				touchwin(win);
				wrefresh(win);
				wrefresh(win1);
				flag = 0;
			}
		}
	}
	end:
	unpost_form(form);
	free_form(form);
	for(i = 0; i < 7;i++) {
		free_field(field[i]);
	}
	wmove(win, 2, 1);
	wclrtobot(win);
	wmove(win, 1, 3);
	wrefresh(win);
}
void invoice(WINDOW *win) {
	MENU *menu3;
	ITEM **item;
	char *item_names[] = {"New Invoice", "View Previous Invoices"};
	int i = 0, size;
	size = size(item_names);
	
	init_pair(41, COLOR_WHITE, COLOR_MAGENTA);
	init_pair(42, COLOR_RED, COLOR_GREEN);
	init_pair(43, COLOR_BLACK, COLOR_WHITE);
	
	item = (ITEM**)malloc( (size + 1) * sizeof(ITEM*));
	while(i < size) {
		item[i] = new_item(item_names[i], item_names[i]);
		i++;
	}
	item[i] = (ITEM*)NULL;
	menu3 = new_menu(item);
	menu_opts_off(menu3, O_SHOWDESC);
	set_menu_win(menu3, win);
	set_menu_sub(menu3, derwin(win, 1,  60, 1, 3));
	set_menu_fore(menu3, COLOR_PAIR(42));
	set_menu_back(menu3, COLOR_PAIR(41));
	set_menu_format(menu3, 1, size);
	set_menu_mark(menu3, ">");
	
	post_menu(menu3);
	wrefresh(win);
	
	while(i = wgetch(win)) {
		switch(i) {
			case KEY_RIGHT:
				menu_driver(menu3, REQ_NEXT_ITEM);
				break;
			case KEY_LEFT:
				menu_driver(menu3, REQ_PREV_ITEM);
				break;
			case 27:
				goto end;
				break;
			case 10:
				if(!strcmp("New Invoice", item_name(current_item(menu3))) ) {
					newinvoice(win);
				}
				box(win, 0, 0);
		}
		wrefresh(win);
	}
	end:
	unpost_menu(menu3);
	free_menu(menu3);
	free_item(item[0]);
	free_item(item[1]);
	free(item);
}
void profile(WINDOW *win) {
	FIELD **field;
	FORM *form;
	int i = 0, x, y, size;
	char *field_names[] = {"Proprietor Name", "Store Name", "Reg. No", "Address", "Save" };
	size = size(field_names);
	
	init_pair(31, COLOR_BLACK, COLOR_WHITE);
	init_pair(32, COLOR_BLACK, COLOR_WHITE);
	init_pair(33, COLOR_RED, COLOR_GREEN);
	
	field = (FIELD**)malloc((size + 1) * sizeof(FIELD*));
	while(i < size - 1) {
		mvwprintw(win, 3 + 2 * i, 4, "%s : ", field_names[i]);
		field[i] = new_field(1, 30, 2 + 2 * i, 1, 0, 0);
		field_opts_off(field[i], O_AUTOSKIP);
		set_field_back(field[i], COLOR_PAIR(32));
		set_field_fore(field[i], COLOR_PAIR(32));
		i++;
	}
	field[i] = new_field(1, 6, 2 + 2 * i, 10, 0, 0);
	field_opts_off(field[i], O_AUTOSKIP | O_EDIT);
	set_field_back(field[i], COLOR_PAIR(33));
	set_field_fore(field[i], COLOR_PAIR(33));
	set_field_buffer(field[i], 0, " Save ");
	field[++i] = NULL;
	
	form = new_form(field);
	scale_form(form, &y, &x);
	
	set_form_win(form, win);
	set_form_sub(form, derwin(win, y, x, 1, 25));
	
	post_form(form);
	wrefresh(win);
	
	while(i = wgetch(win)) {
		switch(i) {
			case KEY_DOWN:
				form_driver(form, REQ_NEXT_FIELD);
				form_driver(form, REQ_END_LINE);
				break;
			case KEY_UP:
				form_driver(form, REQ_PREV_FIELD);
				form_driver(form, REQ_END_LINE);
				break;
			case 27:
				goto end;
				break;
			case KEY_BACKSPACE: case 127:
				form_driver(form, REQ_DEL_PREV);
				break;
			case KEY_DC:
				form_driver(form, REQ_DEL_CHAR);
				break;
			default:
				form_driver(form, i);
				break;
			
		}
	}
	end:
	unpost_form(form);
	free_form(form);
	i = 0;
	while(i < size) {
		wmove(win, 3 + 2 * i, 1);
		wclrtoeol(win);
		free_field(field[i]);
		i++;
	}
	free(field);
	box(win, 0, 0);
	wrefresh(win);
	
}
void inventory(WINDOW *win) {
	ITEM **item;
	MENU *menu2;
	WINDOW *menu2win, *subwin1, *subwin2;
	FIELD **field = (FIELD**)malloc(3 * sizeof(FIELD*));
	FORM *form;
	int i = 0, x, y;
	char *options[] = {"Name", "Category", "Batch No", "Location", "Expiry Date", "Quantity"};
	char *item_names[] = {"Sr No", "Name", "Company", "Category", "Batch No.", "Location", "Expiry Date", "Quantity", "Price"};
	int size = size(options);
	int location[] = {1, 7, 39, 60, 72, 85, 98, 113, 125};
	char a[31];
	
	getmaxyx(win, y, x);
	init_pair(21, COLOR_BLACK, COLOR_WHITE);
	init_pair(22, COLOR_WHITE, COLOR_MAGENTA);
	init_pair(23, COLOR_RED, COLOR_GREEN);
	
	menu2win = newwin(8, COLS - 4, 9, 2);
	box(menu2win, 0, 0);
	wbkgd(menu2win, COLOR_PAIR(22));
	while( i < size(location) ) {
		mvwprintw(menu2win, 6, location[i], "%s",item_names[i]);
		mvwaddch(menu2win, 6, location[i] - 1, ACS_VLINE);
		i++;
	}
	mvwhline(menu2win, 5, 1, ACS_HLINE, x - 2);
	mvwaddch(menu2win, 7 , 0, ACS_LTEE);
	mvwaddch(menu2win, 7, x - 1, ACS_RTEE);
	
	item = (ITEM**)malloc((size + 1) * sizeof(ITEM*));
	i = 0;
	while(i < size) {
		item[i] = new_item(options[i], "");
		i++;
	}
	item[i] = (ITEM*)NULL;
	menu2 = new_menu((ITEM**)item);
	keypad(menu2win, TRUE);
	menu_opts_off(menu2, O_SHOWDESC);
	
	subwin1 = derwin(menu2win, 1, COLS - 10, 2, 2);
	set_menu_win(menu2, menu2win);
	set_menu_sub(menu2, subwin1);
	set_menu_format(menu2, 1, size);
	set_menu_mark(menu2, ">");
	set_menu_fore(menu2, COLOR_PAIR(23));
	set_menu_back(menu2, COLOR_PAIR(22));
	
	post_menu(menu2);
	mvwprintw(menu2win, 1, 2, "Select Search Criteria ->"); 
	
	field[0] = new_field(1, 30, 0, 0, 0, 0);
	field[1] = NULL;
	
	set_field_back(field[0], COLOR_PAIR(21));
	set_field_fore(field[0], COLOR_PAIR(21));
	field_opts_off(field[0], O_AUTOSKIP);
	form = new_form(field);
	
	subwin2 = derwin(menu2win, 1, 60, 4, 2);
	set_form_win(form, menu2win);
	set_form_sub(form, subwin2);
	post_form(form);
	
	print_items(win, location);
	wrefresh(win);
	
	while(i = wgetch(menu2win)) {
		switch(i) {
			case KEY_RIGHT:
				menu_driver(menu2, REQ_RIGHT_ITEM);
				break;
			case KEY_LEFT:
				menu_driver(menu2, REQ_LEFT_ITEM);
				break;
			case 10://Enter key
				mvwprintw(menu2win, 3, 2, "Enter %s :- ", item_name(current_item(menu2)));
				a[0] = '\0';
				wmove(menu2win, 4, 2);
				while( i = wgetch(menu2win)) {
					if(i == 27) {
						set_field_buffer(field[0], 0, "");
						break;
					}
					else if(i == KEY_LEFT) 
						form_driver(form, REQ_PREV_CHAR);
					else if(i == KEY_RIGHT) 
						form_driver(form, REQ_NEXT_CHAR);
					else if( i == 127 || i == KEY_BACKSPACE ) {
						getyx(menu2win, y, x);
						form_driver(form, REQ_DEL_PREV);
						swap_fore(x - 2, a);
						search_inventory(win, item_name(current_item(menu2)), a, location);
					}
					else if(i == KEY_DC) {
						getyx(menu2win, y, x);
						form_driver(form, REQ_DEL_CHAR);
						swap_fore(x - 1, a);
						search_inventory(win, item_name(current_item(menu2)), a, location);
					}
					else {
						getyx(menu2win, y, x);
						form_driver(form, i);
						swap_back(x - 2, a);
						a[x - 2] = i;
						search_inventory(win, item_name(current_item(menu2)), a, location);
					}
					//box(win, 0, 0);
					wrefresh(win);
					//wrefresh(menu2win);
				}
				break;
			case 27://Escape key
				goto end;
				break;
			
		}
		wmove(menu2win, 3, 1);
		wclrtoeol(menu2win);
		wrefresh(menu2win);
	}
	
	end:
	wborder(menu2win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	unpost_menu(menu2);
	free_menu(menu2);
	i = 0;
	while (i <= size) {
		wmove(menu2win, i, 1);
		wclrtoeol(menu2win);
		free_item(item[i]);
		i++;
	}
	unpost_form(form);
	free_form(form);
	free_field(field[0]);
	free_field(field[1]);
	wrefresh(menu2win);	
	delwin(menu2win);
	werase(win);
	wrefresh(win);	
}

void newitem(WINDOW *formwin) {
	FIELD **field;
	FORM *form;
	int i = 0, x, y, size, flag;
	char *field_names[] = {"Name", "Company", "Description", "Category", "Batch No.", "Location", "Expiry Date", "Quantity", "Price"};
	char *emptystr = "                              ";
	size = size(field_names);
	init_pair(11, COLOR_RED, COLOR_BLACK);
	init_pair(12, COLOR_BLACK, COLOR_WHITE);
	init_pair(13, COLOR_WHITE, COLOR_MAGENTA);
	init_pair(16, COLOR_RED, COLOR_GREEN);
	
	field = (FIELD**)malloc((size + 2) * sizeof(FIELD*));
	
	while(i < size) {
		mvwprintw(formwin, 3 + 2 * i, 4, "%s : ", field_names[i]);
		field[i] = new_field(1, 30, 2 + 2 * i, 1, 0, 0);
		field_opts_off(field[i], O_AUTOSKIP);
		set_field_back(field[i], COLOR_PAIR(12));
		set_field_fore(field[i], COLOR_PAIR(12));
		i++;
	}
	field[i] = new_field(1, 5, 3 + 2 * i, 13, 0, 0);
	field_opts_off(field[i], O_AUTOSKIP | O_EDIT);
	set_field_back(field[i], COLOR_PAIR(16));
	set_field_fore(field[i], COLOR_PAIR(16));
	set_field_buffer(field[i], 0, " Add ");
	field[++i] = NULL;
	form = new_form(field);
	scale_form(form, &y, &x);
	
	set_form_win(form, formwin);
	set_form_sub(form, derwin(formwin, y, x, 1, 18));
	
	post_form(form);
	wrefresh(formwin);
	
	while(i = wgetch(formwin)) {
		wmove(formwin, 24, 1);
		wclrtoeol(formwin);
		if(isprint(i) || (i == ' ') || (i == '\t') )
			form_driver(form, i);
		else {
			switch(i) {
				case KEY_DOWN:
					form_driver(form, REQ_NEXT_FIELD);
					form_driver(form, REQ_END_LINE);
					break;
				case KEY_UP:
					form_driver(form, REQ_PREV_FIELD);
					form_driver(form, REQ_END_LINE);
					break;
				case KEY_LEFT:
					form_driver(form, REQ_PREV_CHAR);
					break;
				case KEY_RIGHT:
					form_driver(form, REQ_NEXT_CHAR);
					break;
				case 27:
					goto end;
					break;
				case KEY_BACKSPACE: case 127:
					form_driver(form, REQ_DEL_PREV);
					break;
				case KEY_DC:
					form_driver(form, REQ_DEL_CHAR);
					break;
				case 10:
					if(field[9] == current_field(form)) {
						flag = 0;
						i = 0;
						while(i < 9) {
							if(strcmp(field_buffer(field[i], 0), emptystr) == 0 && i != 2 && i != 5) {
								mvwprintw(formwin, 24, 8, "The field %s cannot be empty ", field_names[i]);
								flag = 1;
								break;
							}
							i++;
						}
						if(flag == 0) {
							i = newitem_process(form, field);
							if( i == 0 )
								mvwprintw(formwin, 24, 8, "Error Occured in saving ");
							else {
								mvwprintw(formwin, 24, 8, "Data saved Successfully");
								i = 0;
								while( i < size ) 
									set_field_buffer(field[i++], 0, "");
							}
						}
					}	
					else {
						form_driver(form, REQ_NEXT_FIELD);
						form_driver(form, REQ_END_LINE);
					}
					break;
				default:
					getyx(formwin, y, x);
					mvwprintw(formwin, 24, 8, "Character not allowed");
					wmove(formwin, y, x);
					break;
			}
		}
	}
	end:
	unpost_form(form);
	free_form(form);
	i = 0;
	while(i < size) {
		wmove(formwin, 3 + 2 * i, 1);
		wclrtoeol(formwin);
		free_field(field[i]);
		i++;
	}
	free(field);
	box(formwin, 0, 0);
	wrefresh(formwin);
}

int main() {
	WINDOW *menu1win, *mainwin;
	ITEM **item;
	MENU *menu1;
	int ch;
	char *title = "Medical Inventory 1.0";
	int i = 0, size;
	char *main_menu[] = {"Home", "Add New item", "Inventory", "Invoice", "Profile", "Exit" };
	
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	start_color();
	init_pair(1, COLOR_RED, COLOR_GREEN);
	init_pair(2, COLOR_RED, COLOR_WHITE);
	init_pair(3, COLOR_WHITE, COLOR_MAGENTA);
	init_pair(4, COLOR_GREEN, COLOR_BLACK);
	init_pair(5, COLOR_BLACK, COLOR_WHITE);
	init_pair(6, COLOR_BLACK, COLOR_YELLOW);
	
	attron(A_BOLD);
	mvprintw(1, (COLS - strlen(title) ) / 2, title);
	attroff(A_BOLD);
	bkgd(COLOR_PAIR(5));
	refresh();
	
	mainwin = newwin(LINES - 11, COLS - 4, 9, 2);
	wbkgd(mainwin, COLOR_PAIR(3));
	box(mainwin, 0, 0);
	keypad(mainwin, TRUE);
	scrollok(mainwin, TRUE);
	wrefresh(mainwin);
	
	menu1win = newwin(5, COLS - 4, 3, 2);
	keypad(menu1win, TRUE);
	wbkgd(menu1win, COLOR_PAIR(3));
	box(menu1win, 0, 0);
	
	size = size(main_menu);
	item = ( ITEM** )malloc((size + 1) * sizeof(ITEM*));
	while(i < size) {
		item[i] = new_item(main_menu[i], main_menu[i]);
		i++;
	}
	item[i] = (ITEM*)NULL;
	menu1 = new_menu((ITEM**)item);
	menu_opts_off(menu1, O_SHOWDESC);
	
	set_menu_win(menu1, menu1win);
	set_menu_sub(menu1, derwin(menu1win, 1, COLS - 10, 2, 4));
	set_menu_format(menu1, 1, size);
	set_menu_mark(menu1, "*"); 
	
	set_menu_fore(menu1, COLOR_PAIR(1));
	set_menu_back(menu1, COLOR_PAIR(3));
	
	post_menu(menu1);
	wrefresh(menu1win);
	
	while(ch = wgetch(menu1win)) {
		switch(ch) {
			case KEY_RIGHT:
				menu_driver(menu1, REQ_RIGHT_ITEM);
				break;
			case KEY_LEFT:
				menu_driver(menu1, REQ_LEFT_ITEM);
				break;
			case 10:
				if(strcmp("Exit",item_name(current_item(menu1))) == 0) {
					size = size(main_menu);
					unpost_menu(menu1);
					free_menu(menu1);
					i = 0;
					while(i < size + 1) {
						free_item(item[i]);
						i++;
					}
					endwin();
					return 0;
				}
				if(strcmp("Add New item", item_name(current_item(menu1))) == 0) {
					newitem(mainwin);
				}
				if(strcmp("Inventory", item_name(current_item(menu1))) == 0) {
					inventory(mainwin);
					move(LINES - 2, 1);
					clrtoeol();
					box(mainwin, 0, 0);
					wrefresh(mainwin);
				}
				if(strcmp("Profile", item_name(current_item(menu1))) == 0) {
					profile(mainwin);
					box(mainwin, 0, 0);
					wrefresh(mainwin);
				}
				if(strcmp("Invoice", item_name(current_item(menu1))) == 0) {
					invoice(mainwin);
					box(mainwin, 0, 0);
					wrefresh(mainwin);
				}
		}
		refresh();
	}
	
	unpost_menu(menu1);
	free_menu(menu1);
	i = 0;
	while(i < size)
		free_item(item[i++]);
	free(item);
	endwin();
	return 0;
}

