#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//Прототипы функций
int size_of_largest_word (char*);
char* delete_enters (char*);
int find_prev_space (char*);
int find_next_space (char*);
int stroks_counter (char*);
int stroks_counter_2 (char*);
char* stroks_creator (char*, char**);
char* stroks_creator_2 (char*, char**);
char* stroks_creator_break (char*, char**);
void write_result (char**, FILE*);
void help(int, void*);
int iscorrect(char*, long);
char* read_incoming_text (FILE*);

//Глобальные переменные
int strok_len;
int column_height;
int stroks_count;

//"Булевые" переменные
int bool_break_words = 0;		//можно ли разрывать слова?
int bool_read_from_file = 1;	//читать ли данные из файла или из потока ввода?
int bool_write_to_file = 1;		//выводить данные в файл или поток вывода?

//Массивы данных
char* pre_string;
char* string;
char simvols[] = {'\t', '\n', ' '};	//массив табуляции

//Файлы ввода-вывода
FILE *input_file;	//Файл с исходным текстом
FILE *output_file;	//Файл с результатом работы программы

//Опции программы
long c;
long w;
int columns_count;
int columns_width;



//ТЕЛО ПРОГРАММЫ//
int main(int argc, char *argv[])	{


	on_exit(help, NULL);
	
	
	//Проверка корректности запуска 
	if (argc < 5 || argc > 8) exit(1);
  
	if	(strcmp(argv[1], "-c") != 0 ||
		strcmp(argv[3], "-w") != 0) 
		exit(1);
    
	if	(sscanf(argv[2], "%ld", &c) != 1 ||
		sscanf(argv[4], "%ld", &w) != 1)
		exit(2);
	
	if (!iscorrect(argv[2], c)) exit(3);
	if (!iscorrect(argv[4], w)) exit(3);
	
	if (c <= 0) exit(3);
	
	
	//Выставляем "булевые" переменные
	if (argc > 5)	{	//такая проверка нужна, чтобы программа не пыталась считать несуществующую шестую опцию
		if (strcmp(argv[5], "-r") == 0)	bool_break_words = 1;	//разрывать ли слова
	}

	if (argc < 6) bool_read_from_file = 0;	//определяемся с источником ввода
	if (argc == 6 && bool_break_words == 1) bool_read_from_file = 0;
	
	if (argc < 7) bool_write_to_file = 0;	//определяемся с назначением вывода
	if (argc == 7 && bool_break_words == 1) bool_write_to_file = 0;	
	
	//задаем источник ввода и назначение вывода
	if (bool_read_from_file == 0) input_file = stdin;
	if (bool_read_from_file == 1 && bool_break_words == 0) {
		input_file = fopen (argv[5], "r");
		if (input_file == NULL) exit(4);
	}
	if (bool_read_from_file == 1 && bool_break_words == 1) {
		input_file = fopen (argv[6], "r");
		if (input_file == NULL) exit(4);	
	}	
	
	if (bool_write_to_file == 0) output_file = stdout;  
	if (bool_write_to_file == 1 && bool_break_words == 0) {output_file = fopen(argv[6], "w");}
	if (bool_write_to_file == 1 && bool_break_words == 1) {output_file = fopen(argv[7], "w");}


	//Считываем и обрабатываем исходную информацию
	pre_string = read_incoming_text(input_file);	//создание первичного массива с исходным текстом	
	//printf("\npre_string: %s\n", pre_string);
	
	string = malloc(BUFSIZ);	
	sprintf(string, delete_enters(pre_string), BUFSIZ);	//заменяем знаки переноса строки на пробелы
	
	free(pre_string);	//освобождаем память из под pre_string
	
	//printf("DELETED ENTERS STRING: %s\n", string);
	
	
	//Приводим значения из опций в более безопасный тип int
	columns_count = (int)c;
	//printf("columns_count = %d\n", columns_count);
	columns_width = (int)w;
	//printf("columns_width = %d\n", columns_width);
	
	
	//Определение парметров будущего двумерного массива БЕЗ РАЗРЫВА СЛОВ
	if (bool_break_words == 0)	{
			
			//Определение количества строчек
			stroks_count = stroks_counter(string);
			//printf("stroks_count: %d\n", stroks_count);
			
			//Определение высоты столбцов в строчках
			if (stroks_count % columns_count == 0) {column_height = stroks_count / columns_count;}
			else {column_height = (stroks_count / columns_count) + 1;};
			//printf("column_height: %d\n", column_height);
			
			//Определение длины строчки в символах
			if (columns_width > size_of_largest_word(string))	{strok_len = columns_width;}
			else	{strok_len = size_of_largest_word(string);}
			//printf("strok_len: %d\n", strok_len);	
	
	}
	
	//Определение парметров будущего двумерного массива С РАЗРЫВОМ СЛОВ
	if (bool_break_words == 1)	{
			
			//Определение количества строчек
			if (strlen(string) % columns_width == 0) {stroks_count = strlen(string) / columns_width;}
			else {stroks_count = (strlen(string) / columns_width) + 1;}
			
			//Определение высоты столбцов в строчках
			if (stroks_count % columns_count == 0) {column_height = stroks_count / columns_count;}
			else {column_height = (stroks_count / columns_count) + 1;};
			
			//Определение длины строчки в символах
			strok_len = columns_width;
	}


	//Создание двумерного массива для занесения новых данных
	char **stroks = (char **)calloc((stroks_count + 1) , sizeof(char *));
	if (stroks == NULL) exit(5);
	for (int i=0; i < stroks_count; i++)
		{stroks[i] = (char *)calloc(strok_len , sizeof(char));}
		if (stroks == NULL) exit(5);


	//Заполнение двумерного массива без разрыва слов
	if (bool_break_words == 0)	{stroks_creator_2 (string, stroks);}
	//Заполнение двумерного массива с разрывом слов
	if (bool_break_words == 1)	{stroks_creator_break (string, stroks);}

	
	//Вывод данных из второго массива в конечный файл
	if (bool_write_to_file == 0) printf("\nResult:\n");
	write_result(stroks, output_file);
	

	//Освобождаем память
	free(string);
	free(stroks);
	
}






//ФУНКЦИИ//

void write_result (char** array, FILE* f)	{
	for (int str=0; str<column_height; )	{
		int r = str;
		for (int stl=0; stl<c; stl++)	{
			if (r < stroks_count)	{
				
				int buff = strok_len - strlen(array[r]);
				fwrite(array[r], sizeof(char), strlen(array[r]), f);
				if (bool_write_to_file == 0) {for (int i=0; i<buff; i++) {fwrite(&simvols[2], sizeof(char), buff, f);}}
				fwrite(&simvols[0], sizeof(char), 1, f);
				fwrite(&simvols[0], sizeof(char), 1, f);
				
				r += column_height;
			}
		}
		fwrite(&simvols[1], sizeof(char), 1, f);
		str++;
	}
	fclose(f);
}


char* read_incoming_text (FILE *f)	{
	char *array;
	char *array_kon;
	long number_of_symbols = 0;
	long size_of_file = sizeof(char);
	
	array = malloc(size_of_file);
	if (array == NULL) exit(5);

	do {	
		array_kon = array + number_of_symbols++;
		*array_kon = fgetc(f);
		array = realloc (array, (size_of_file += sizeof(char)));
		if (array == NULL) exit(5);
	} while (!feof(f)); 
	fclose(f);
	
	//printf("PRE_ARRAY: %s\n", array);
	//Костыль, избавляемся от лишнего символа в конце
	char *result_array = malloc(strlen(array) * sizeof(char));
	if (result_array == NULL) exit(5);
	memcpy(result_array, array, strlen(array) - 1);
	
	free(array);
	return result_array;
}


void help (int err, void *arg) {
	
	switch(err) {
		case 1:
			fprintf(stderr, "Incorrect launch: wrong options!\nSyntax: program -c (number_of_columns) -w (column_width) -r (optional:break_words) [optional:incoming_file [optional:resulting_file]]\n");
			break;			
		case 2:
			fprintf(stderr, "Incorrect launch: wrong type of options!\nSyntax: program -c (number_of_columns) -w (column_width) -r (optional:break_words) [optional:incoming_file [optional:resulting_file]]\n");
			break;		
		case 3:
			fprintf(stderr, "Incorrect launch: wrong value of options!\nSyntax: program -c (number_of_columns) -w (column_width) -r (optional:break_words) [optional:incoming_file [optional:resulting_file]]\n");
			break;
		case 4:
			fprintf(stderr, "Can't open file!\n");
			break;
		case 5:
			fprintf(stderr, "Failure attemp to create array\n");
			break;	
	}    
}


int iscorrect(char *array, long f)	{
	char p[10];
	sprintf(p, "%ld", f);
	if (strlen(p) == strlen(array)) return 1;
	return 0;
}


char* delete_enters (char* array)	{
	char* a = malloc(strlen(array));
	sprintf(a, array, strlen(array));			
	for (int i=0; i < strlen(a); i++)	{
		char bukva = a[i];
		//printf("bukva: %c\n", bukva);
		if (bukva=='\n') {
		a[i] = ' ';}
	}
	return a;
}


int size_of_largest_word (char* array)	{
	int wl=0; int wl_max=0;
	for (int i=0; i < strlen(array); i++)	{
		char bukva = array[i];
		if (!isspace(bukva)) {wl++;}
		else {
			if (wl>wl_max) {wl_max=wl;}
			wl = 0;
		}
	}
	return wl_max;
}


int find_next_space (char* kon)	{
	int next_space_in = 0;
	while (*kon != ' ') {	
		kon += sizeof(char);
		next_space_in++;
	}
	return next_space_in;
}


int find_prev_space (char* kon)	{
	int prev_space_in = 0;
	while (*kon != ' ') {	
		kon -= sizeof(char);
		prev_space_in++;
	}
	return prev_space_in;
}


int stroks_counter (char* array)	{
	int counter = 0;
	char* nach = &array[0];
	char* kon = nach;
	char* true_kon = &array[strlen(array)];
	int w = columns_width;
	
	
	while (nach + w < true_kon)	{
		kon = nach + w;
			
			if (*kon == ' ')	{
				counter++;
				nach = kon + sizeof(char);
			}
			if (*kon != ' ')	{
				
				if (w - find_prev_space(kon) <= 0)	{
					counter++;
					nach = kon + find_next_space(kon) + sizeof(char);
				}
				else	{
					counter++;
					nach = kon - find_prev_space(kon) + sizeof(char);
				}
			}
		}
			
		if (nach + w >= true_kon)	{
			if (true_kon - nach > 0) counter++;			
		}
			
	return counter;
}


int stroks_counter_2 (char* array)	{
	
	int stroks_counter = 1;
	char* nach;
	char* kon;
	char* true_kon = &array[strlen(array)];
	nach = &array[columns_width];
	kon = nach;
	
	while (kon < true_kon)	{

		//printf("CS: nach: %s\n", nach);
		//printf("CS: kon: %s\n", kon);
		//printf("CS: stroks_counter %d\n", stroks_counter);

		if ((*kon == ' ') || (*kon + sizeof(char) == ' '))	{
			stroks_counter++;
			nach = kon;
			
			}
		else	{
			if ((columns_width - find_prev_space(kon)) <= 0)	{
				nach = kon + sizeof(char)*(find_next_space(kon)) + sizeof(char);
				stroks_counter++;
			}
			else	{
				nach = kon - sizeof(char)*(find_prev_space(kon)) + sizeof(char);
				stroks_counter++;
				//printf("kon - NE probel! stroks_counter: %d\n", stroks_counter);
				//printf("new(ne probel) kon: %s\n", kon);
			}
		}
		if (nach + columns_width >= true_kon) break;
		kon = nach + columns_width*sizeof(char);
		//printf("CE: kon: %s\n", kon);
		//printf("CE: nach: %s\n\n", nach);
	}
	return stroks_counter;
}


char* stroks_creator (char* array1, char** array2)	{
	
	int y = 0;
	char* nach;
	char* kon;
	char* true_kon = &array1[strlen(array1)];
	nach = &array1[0];
	//kon = &array1[strok_len];
	int dlina;

	if (*nach == ' ')	{
		while (*nach != ' ') nach += sizeof(char);
	}
	kon = nach + strok_len;
	//printf("nach: %s\n", nach);
	//printf("kon: %s\n", kon);
	//printf("true_kon: %s\n", true_kon - 1);
	
	while (y < (stroks_count - 1))	{
		if ((*kon == ' ') || (*kon + sizeof(char) == ' '))	{
			//printf("kon = probel\n");
			dlina = kon - nach;
			//printf("dlina: %d\n", dlina);
			if (abs(dlina) <= strok_len*2)	{	//Защита от выхода за границы массива при копировании
				memcpy (&array2[y][0], nach, dlina);
				//printf("\t\tarray2[%d]: %s\n", y, &array2[y][0]);
				nach = kon + sizeof(char);
			}
		}
		else	{
			//printf("kon != probel\n");			
			if ((w - find_prev_space(kon)) <= 0)	{
				kon = kon + sizeof(char)*(find_next_space(kon)) + sizeof(char);
				//printf("Пробел спереди! kon: %s\n", kon);
				dlina = kon - nach;
				//printf("\tdlina: %d\n", dlina);
				if (abs(dlina) <= strok_len*2)	{	//Защита от выхода за границы массива при копировании
					memcpy (&array2[y][0], nach, dlina);
					//printf("\t\tarray2[%d]: %s\n", y, &array2[y][0]);
					nach = kon;	
				}
			}
			else{
				kon = kon - sizeof(char)*(find_prev_space(kon)) + sizeof(char);
				//printf("Пробел сзади! nach: %s\n", nach);
				//printf("Пробел сзади! kon: %s\n", kon);
				dlina = kon - nach;
				//printf("\tdlina: %d\n", dlina);
				if (abs(dlina) <= strok_len*2)	{	//Защита от выхода за границы массива при копировании
					memcpy (&array2[y][0], nach, dlina);
					//printf("\t\tarray2[%d]: %s\n", y, &array2[y][0]);
					nach = kon;
				}
			}
		}
		kon = nach + strok_len*sizeof(char);
		//printf("В конце цикла nach: %s\n", nach);
		//printf("В конце цикла kon: %s\n", kon);
		y++;
	}
	if (y == (stroks_count - 1))	{	//Обработка последнего кусочка текста	
		kon = true_kon;
		//printf("Последняя строка! nach: %s\n", nach);
		//printf("Последняя строка! kon: %s\n", kon);
		dlina = kon - nach;
		//printf("\tdlina: %d\n", dlina);
		if (abs(dlina) <= strok_len*2)	{	//Защита от выхода за границы массива при копировании
			memcpy (&array2[y][0], nach, dlina);
			//printf("\t\tarray2[%d]: %s\n", y, &array2[y][0]);
			nach = kon;
		}
	}	
	return *array2;
}


char* stroks_creator_2 (char* array1, char** array2)	{
	
	int y = 0;
	char* nach;
	char* kon;
	char* true_kon = &array1[strlen(array1)];
	nach = &array1[0];
	kon = nach;
	int dlina;
	if (*nach == ' ')	{
		while (*nach != ' ') nach += sizeof(char);
	}
	
	while (nach + strok_len < true_kon)	{
		
		kon = nach + strok_len;
		
		if (*kon == ' ')	{
			dlina = kon - nach;
			memcpy (&array2[y][0], nach, dlina);
			nach = kon + sizeof(char);
		}
		else	{			
			if (strok_len - find_prev_space(kon) <= 0)	{
				kon = kon + sizeof(char)*(find_next_space(kon));
				dlina = kon - nach;
				memcpy (&array2[y][0], nach, dlina);
				nach = kon + find_next_space(kon) + sizeof(char);
			}
			else{
				kon = kon - sizeof(char)*(find_prev_space(kon));
				dlina = kon - nach;
				memcpy (&array2[y][0], nach, dlina);
				nach = kon - find_prev_space(kon) + sizeof(char);	
			}
		}
		y++;
	}
	
	if (nach + w >= true_kon)	{
			if (true_kon - nach > 0) {
				kon = true_kon;
				dlina = kon - nach;
				memcpy (&array2[y][0], nach, dlina);
			}
	}
	return *array2;
}


char* stroks_creator_break (char* array1, char** array2)	{
		char* nach = &array1[0];
		char* kon = nach;
		char* true_kon = &array1[strlen(array1)];
		int dlina;
		for (int y=0; y < stroks_count; y++) {
			for (int i=0; i<strok_len; i++)	{
				if (kon != true_kon) kon += sizeof(char);
			}	
		dlina = kon - nach;
		memcpy (&array2[y][0], nach, dlina);
		nach = kon;
		}
}
