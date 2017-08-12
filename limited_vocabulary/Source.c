#include <stdio.h>
#include <string.h>

#define SECCESS 0
#define WRONG_ARGUMENTS -1
#define CANNOT_OPEN_TEXT_FILE -2
#define CANNOT_OPEN_VOCABULARY_FILE -3
#define CANNOT_CLOSE_TEXT_FILE -4
#define CANNOT_CLOSE_VOCABULARY_FILE -5
#define ERROR_OUT_OF_MEMORY -6
#define ERROR_SEEK -7
#define ERROR_UPDATE_TEXT -8


#define TRUE 1
#define FASLE 0

#define THERE_IS_ON_SYNONYMS " "
#define TEMP_FILE_NAME "tempFile.txt"
/*
vocabularyFile protocol:
in every line there is words that are synonyms.
every new line - new synonyms.
e.g. - vocabularyFile.txt:

drink beverage
persecution oppression synonymousness
self me
*/


typedef struct
{
	char* word1;
	char* word2;
	int isLastPairInLine;
}pairWords;

typedef struct
{
	char*** vocabulary;
	int numberOfWords;
}vocabulary;

pairWords next_two_pairs(FILE* vocabularyFile);
/* Method to allocate a 3D array of char, and init them.*/
char*** make_3d_array(int numberOfWords, FILE* vocabularyFile);
int get_number_of_words(FILE * vocabularyFile);
/*BinarySearch to the wanted word.
If find return one of this synonym.*/
char * get_synonym(const char * temp, vocabulary sortedVocabulary);
void update_text(FILE* emptyTextFile, FILE* textFile, vocabulary sortedVocabulary);
/*copy date from one file to other one. */
int copy_file_to_file(FILE * copyFrom, FILE * copyTo);
/*hepler func for qsort*/
int cmpfunc(const void * a, const void * b);

int main(int agrc, char* agrv[])
{
	FILE * textFile = NULL;
	FILE * vocabularyFile = NULL;
	FILE * tempFile = NULL;
	int valToReturn;
	int i;	

	int numberOfWords = 0;
	char*** vocabularyWords = NULL;
	
	if (agrc != 3)
	{
		printf("Wrong arguments, format: limitedVocabulary.exe textFileName.txt vocabularyFileName.txt\n\n");
		system("PAUSE");
		return WRONG_ARGUMENTS;
	}
	textFile = fopen(agrv[1], "r");
	if (textFile == NULL)
	{
		printf("Cannot open text file.\n\n");
		system("PAUSE");
		return CANNOT_OPEN_TEXT_FILE;
	}
	/* Will write the textFile to this file.*/	
	tempFile = fopen(TEMP_FILE_NAME, "w");
	if (tempFile == NULL)
	{
		printf("Cannot open vocabulary file.\n\n");
		system("PAUSE");
		return ERROR_UPDATE_TEXT;
	}

	copy_file_to_file(textFile, tempFile);
	fclose(textFile);
	/*Open textFile as new file*/
	textFile = fopen(agrv[1], "w");
	if (textFile == NULL)
	{
		printf("Cannot open text file.\n\n");
		system("PAUSE");
		return CANNOT_OPEN_TEXT_FILE;
	}

	vocabularyFile = fopen(agrv[2], "r");
	if (vocabularyFile == NULL)
	{
		printf("Cannot open vocabulary file.\n\n");
		system("PAUSE");
		return CANNOT_OPEN_VOCABULARY_FILE;
	}

	numberOfWords = get_number_of_words(vocabularyFile);

	/*Init vocabularyWords and sort him.
	  Every place in the vocabularyWords have to words (e.g. vocabularyWordsp[0]) have words in places (vocabularyWordsp[0][0] and vocabularyWords[0][1]),
	  There are synonyms words.*/
	vocabularyWords = make_3d_array(numberOfWords, vocabularyFile);
 	qsort(vocabularyWords, numberOfWords, sizeof(char**), cmpfunc);
	vocabulary sortedVocabulary = { vocabularyWords,numberOfWords };
	fclose(tempFile);

	/*open the tempFile to read to data.*/
	tempFile = fopen(TEMP_FILE_NAME, "r");
	if (tempFile == NULL)
	{
		printf("Cannot open text file.\n\n");
		system("PAUSE");
		return CANNOT_OPEN_TEXT_FILE;
	}

	/*update the textFile with the new vocabulary.*/
	update_text(textFile, tempFile, sortedVocabulary);
	
	fclose(tempFile);
	fclose(textFile);
	fclose(vocabularyFile);
	remove(TEMP_FILE_NAME);
	for (i = 0; i < numberOfWords; i++) {
		free(vocabularyWords[i]);
	}
	free(vocabularyWords);
	system("PAUSE");
	return 0;
}
char*** make_3d_array(int numberOfWords, FILE* vocabularyFile) {
	char*** arr;
	int i, j;

	if (fseek(vocabularyFile, 0, SEEK_SET) == -1)
	{
		printf("ERROR: cannot set seek.\n");
		return ERROR_SEEK;
	}

	arr = (char ***)malloc(numberOfWords * sizeof(char**));

	for (i = 0; i < numberOfWords; i++) {
		arr[i] = (char **)malloc(2 * sizeof(*arr[i]));

		pairWords temp = next_two_pairs(vocabularyFile);
		/*Malloc and Init the synonyms words in the vocabularyFile*/
		arr[i][0] = (char *)malloc(strlen(temp.word1) * sizeof(char));
		arr[i][0] = temp.word1;
		arr[i][1] = (char *)malloc(strlen(temp.word2) * sizeof(char));
		arr[i][1] = temp.word2;

		/*If it's the last Pair, give the last word also his synonyms word.*/
		if (temp.isLastPairInLine == TRUE)
		{
			i++;
			arr[i] = (char **)malloc(2 * sizeof(char*));

			arr[i][0] = (char *)malloc(strlen(temp.word2) * sizeof(char));
			arr[i][0] = temp.word2;
			arr[i][1] = (char *)malloc(strlen(temp.word1) * sizeof(char));
			arr[i][1] = temp.word1;
		}
	}

	return arr;
}
pairWords next_two_pairs(FILE* vocabularyFile)
{
	static char* currWord = "";
	char* lastWord = "";
	char* temp = NULL;
	char c = 0;
	char word[150] = { 0 };
	int wordLen = 0;
	while ((c = fgetc(vocabularyFile)) != EOF)
	{
		if (c == ' ')
		{
			if (currWord == "")
			{
				currWord = temp;
				word[0] = '/0';
				wordLen = 0;
				continue;
			}
			lastWord = currWord;
			currWord = temp;
			return (pairWords) {
				lastWord, currWord, FASLE
			};

		}
		else if (c == '\n')
		{
			lastWord = currWord;
			currWord = "";
			return (pairWords) {
				lastWord, temp, TRUE
			};
		}
		else
		{
			wordLen++;
			word[wordLen - 1] = c;
			word[wordLen] = '\0';
			temp = (char *)malloc(wordLen * sizeof(char));
			strcpy(temp, word);
		}
	}
	return (pairWords) {
		currWord, temp, TRUE
	};

}


char * get_synonym(const char * word, vocabulary sortedVocabulary)
{
	int left = 0;
	int right = sortedVocabulary.numberOfWords - 1;
	int middle;
	if (word == NULL)
		return THERE_IS_ON_SYNONYMS;
		
		while (left <= right) {
			middle = ((left + right) / 2);
			if (strcmp(word, sortedVocabulary.vocabulary[middle][0])<0)
				right = middle - 1;
			else
				if (strcmp(word, sortedVocabulary.vocabulary[middle][0])>0)
					left = middle + 1;
				else
					return sortedVocabulary.vocabulary[middle][1];
		}
		return THERE_IS_ON_SYNONYMS;
	
}

void update_text(FILE*writeFile, FILE* readFile, vocabulary sortedVocabulary)
{
	char c = 0;
	char word[150] = { 0 };
	int wordLen = 0;
	char* temp = NULL;
	char* synonym = NULL;
	
	if (fseek(readFile, 0, SEEK_SET) == -1)
	{
		printf("ERROR: cannot set seek.\n");
		return ERROR_SEEK;
	}
	
	while ((c = fgetc(readFile)) != EOF)
	{
		if (!isalpha(c))
		{
			/*if there is an empy line or unreadable one, just put the char and continue.*/
			if (temp != NULL || temp == "")
			{
				if ((synonym = get_synonym(temp, sortedVocabulary)) != THERE_IS_ON_SYNONYMS)
				{
					fputs(synonym, writeFile);
				}
				else
				{
					fputs(temp, writeFile);
				}
				wordLen = 0;
				/*Need just of the last word in the file, if the word is not in the alphabet (e.g. '.','!'),
				  In that case the if in the end of the func will not put the word in the second time.*/
				strcpy(temp, "");
			}
			fputc(c, writeFile);		
		}
		else
		{
			wordLen++;
			word[wordLen - 1] = c;
			word[wordLen] = '\0';
			temp = (char *)malloc(wordLen * sizeof(char));
			strcpy(temp, word);
		}
	}
	if (temp != NULL || temp=="")
	{
		if ((synonym = get_synonym(temp, sortedVocabulary)) != THERE_IS_ON_SYNONYMS)
		{
			fputs(synonym, writeFile);
		}
		else
		{
			fputs(temp, writeFile);
		}
	}
}

int copy_file_to_file(FILE * copyFrom, FILE * copyTo)
{	
	char content[80];
	while (fgets(content, sizeof(content), copyFrom) != NULL)
	{
		fprintf(copyTo, "%s", content);
	}
}
int get_number_of_words(FILE * vocabularyFile)
{
	int numberOfWords = 0;
	char c = 0;
	while ((c = fgetc(vocabularyFile)) != EOF)
	{
		if (c == ' ' || c == '\n')
		{
			numberOfWords++;
		}
	}
	/*for the last word:*/
	++numberOfWords;

	return numberOfWords;
}
int cmpfunc(const void * a, const void * b)
{
	return strcmp(**(char***)a, **(char***)b);
}