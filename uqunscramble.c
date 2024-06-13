#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <csse2310a1.h>
#include <ctype.h>
#define USAGE_ERROR_CODE 17
#define MIN_LENGTH_ERROR_CODE 15
#define INVALID_LETTER_SET_ERROR_CODE 8
#define TOO_MANY_LETTERS_ERROR_CODE 5
#define INSUFFICIENT_LETTERS_ERROR_CODE 12
#define DICTFILE_OPEN_FAILURE_ERROR_CODE 16
#define USAGE_ERROR                                                            \
    "Usage: uqunscramble [--letters letters] [--dictionary dictfile] "         \
    "[--length-min len]\n"
#define NO_WORDS_ERROR_CODE 4
#define MAX_ARGUMENTS 6
#define DEFAULT_LETTER_COUNT 9
#define DEFAULT_MIN_LENGTH 4
#define MIN_LENGTH_LOWER 3
#define MIN_LENGTH_UPPER 6
#define SET_LENGTH_MAX 11
#define INIT_BUFFER 50
#define ALPHABET_SIZE 26
#define MAX_LEN_BONUS 10

/* arg_check()
 * Ensures arguments and the number o arguments passed into main is even
 * and unique. If not then exit.
 *
 * argC: The number of arguments
 * argv[]: The array of arguments
 */
void arg_check(int argC, char* argv[])
{
    if ((argC % 2 != 0) || argC > MAX_ARGUMENTS) {
        fprintf(stderr, USAGE_ERROR);
        exit(USAGE_ERROR_CODE);
    }

    // Check for duplicate arguments
    for (int i = 1; i < argC - 1; i++) {
        for (int j = i + 1; j < argC; j++) {
            if (strcmp(argv[i], argv[j]) == 0) {
                fprintf(stderr, USAGE_ERROR);
                exit(USAGE_ERROR_CODE);
            }
        }
    }
}

/* letter_set_check()
 * Ensures that if a letter set is provided that it is valid. Else uses the
 * given method to generate a letter set.
 *
 * letters: the set of letters.
 * setLengthP: A pointer to the configured length of the letter set.
 * lengthMin: The configured minimum length for a word.
 * Returns: The letterset if it's valid.
 */
char* letter_set_check(char* letters, int* setLengthP, int lengthMin)
{
    // Use provided library function if user doesn't specify letter set
    if (letters == NULL) {
        letters = (char*)get_random_letters(
                DEFAULT_LETTER_COUNT); // Default 9 as per spec
    }

    // ensure length value is valid
    if (!(MIN_LENGTH_LOWER <= lengthMin && lengthMin <= MIN_LENGTH_UPPER)) {
        fprintf(stderr,
                "uqunscramble: min length value must be between 3 and 6\n");
        exit(MIN_LENGTH_ERROR_CODE);
    }

    // Letter set checking using null terminator
    for (int i = 0; letters[i] != '\0'; i++) {
        *setLengthP += 1;
        if (!isalpha(letters[i])) {
            fprintf(stderr, "uqunscramble: invalid letter set\n");
            exit(INVALID_LETTER_SET_ERROR_CODE);
        }
    }

    if (*setLengthP > SET_LENGTH_MAX) {
        fprintf(stderr,
                "uqunscramble: too many letters - at most %d expected\n",
                SET_LENGTH_MAX);
        exit(TOO_MANY_LETTERS_ERROR_CODE);
    } else if (*setLengthP < lengthMin) {
        fprintf(stderr,
                "uqunscramble: insufficient letters for the given minimum "
                "length (%d)\n",
                lengthMin);
        exit(INSUFFICIENT_LETTERS_ERROR_CODE);
    }
    return letters;
}

/* file_check()
 * Ensures that the provided dictionary file can be opened and returns it.
 * If not then the program exists.
 */
FILE* file_check(char* dictPath)
{
    FILE* dictFile = fopen(dictPath, "r");
    if (dictFile == NULL) {
        fprintf(stderr,
                "uqunscramble: dictionary file named \"%s\" cannot be opened\n",
                dictPath);

        exit(DICTFILE_OPEN_FAILURE_ERROR_CODE);
    }
    return dictFile;
}

/* read_line()
 * Dynamically processes input letter by letter from a given FILE* stream.
 * Returns: An uppercased char* of the letters.
 */
char* read_line(FILE* stream)
{
    // Much of this was inspired by Week 3.2 file handling ed lessons which
    // can be used freely without reference according to ed thread #116.
    int bufferSize = INIT_BUFFER;
    char* buffer = malloc(sizeof(char) * bufferSize);
    int numRead = 0;
    int next;

    if (feof(stream)) {
        return NULL;
    }

    while (1) {
        next = fgetc(stream);
        if (next == EOF && numRead == 0) {
            free(buffer);
            return NULL;
        }
        if (numRead == bufferSize - 1) {
            bufferSize *= 2;
            buffer = realloc(buffer, sizeof(char) * bufferSize);
        }
        if (next == '\n' || next == EOF) {
            buffer[numRead] = '\0';
            break;
        }
        buffer[numRead++] = toupper(next);
    }
    return buffer;
}

/* compare_words()
 * A helper method for qsort used to sort words in the dictionary from
 * shortest to longest.
 */
int compare_words(const void* a, const void* b)
{
    // Cast the void pointers to const char** pointers then dereference
    const char* word1 = *(const char**)a;
    const char* word2 = *(const char**)b;
    size_t len1 = strlen(word1);
    size_t len2 = strlen(word2);

    // qsort will use sign and magnitude of return value to determine order
    if (len1 != len2) {
        return len1 - len2;
    }

    return strcmp(word1, word2);
}

/* add_to_array()
 * A method that dynamically creates and populates an array from a given
 * FILE* stream.
 *
 * setLengthP: A pointer to access the configured set length
 * dictFile: The provided file stream
 * numLinesP: A pointer to the variable which helps keep track of the array
 * index.
 *
 * Returns: An array of words sorted from shortest to longest
 *
 */
char** add_to_array(int const* setLengthP, FILE* dictFile, int* numLinesP)
{
    int arraySize = INIT_BUFFER;
    char** words = malloc(arraySize * sizeof(char*));
    char* line = read_line(dictFile);
    while (line != NULL) {
        if (*numLinesP == arraySize - 1) {
            arraySize *= 2;
            words = realloc(words, sizeof(char*) * arraySize);
        }

        int lineLength = (int)strlen(line);
        if (lineLength <= *setLengthP) {
            int alpha = 1;
            // Ensure each letter in the dictionary word is alphabetic
            for (int i = 0; i < lineLength && alpha; i++) {
                if (!(isalpha(line[i]))) {
                    alpha = 0;
                }
            }
            if (alpha) {
                words[(*numLinesP)++] = line;
            } else {
                free(line);
            }
        } else {
            free(line);
        }
        line = read_line(dictFile);
    }
    qsort(words, *numLinesP, sizeof(char*), compare_words);
    fclose(dictFile);
    return words;
}

/* verify_input()
 * Verifies user input by processing it through a range of tests, ensuring it
 * conforms to the min/max length requirements, can be formed from the letterset
 * and can be found in the dictionary.
 *
 * input: the user input
 * lengthMin: The configured minimum length for a word
 * setLengthP: The pointer to the variable storing the length of the letter set
 * letterSetCount A data structure that stores the frequencies of each letter
 * in the letter set.
 * inputWords: A data structure that stores all verified user input
 * numWords: An index variable used to keep track of the words in inputWords
 * words: The dictionary of words
 * numLinesP: A pointer to an index variable user to keep track of the number
 * of words in the dictionary
 *
 * returns: 0 for valid and -1 for invalid
 */
int verify_input(char* input, int lengthMin, const int* setLengthP,
        int const* letterSetCount, char** inputWords, int numWords,
        char** words, const int* numLinesP)
{
    int iLength = (int)strlen(input);
    int inputCount[ALPHABET_SIZE] = {0};
    for (int i = 0; i < iLength; i++) {
        if (!(isalpha(input[i]))) {
            printf("Word must contain only letters\n");
            free(input);
            return -1;
        }
    }
    if (iLength < lengthMin) {
        printf("Word too short - it must have at least %d characters\n",
                lengthMin);
        free(input);
        return -1;
    }
    if (iLength > *setLengthP) {
        printf("Word must have a length of no more than %d characters\n",
                *setLengthP);
        free(input);
        return -1;
    }
    // Count the input letter frequencies
    for (int i = 0; input[i] != '\0'; i++) {
        inputCount[input[i] - 'A']++;
    }
    // Compare letter frequencies
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (inputCount[i] > letterSetCount[i]) {
            printf("Word can't be formed from available letters\n");
            free(input);
            return -1;
        }
    }
    // Check input words dictionary
    for (int i = 0; i < numWords; i++) {
        if (strcmp(inputWords[i], input) == 0) {
            printf("Word has already been guessed\n");
            free(input);
            return -1;
        }
    }
    for (int i = 0; i < *numLinesP; i++) {
        if (strcmp(words[i], input) == 0) {
            return 0;
        }
    }
    printf("Word can't be found in the dictionary\n");
    free(input);
    return -1;
}

/* parse_args()
 * Processes arguments input to the program and attempts to assign variables
 * for game configuration with exit codes for invalid entries.
 */
void parse_args(int argC, char* argv[], char** lettersP, char** dictPathP,
        int* lengthMinP)
{
    // Iterate through args and assign their values
    // Have prechecked for even arg count and uniqueness so it's safe to do
    // i+1
    for (int i = 1; i < argC; i = i + 2) {
        if (strcmp(argv[i], "--letters") == 0) {
            *lettersP = argv[i + 1];
        } else if (strcmp(argv[i], "--dictionary") == 0) {
            *dictPathP = argv[i + 1];
        } else if (strcmp(argv[i], "--length-min") == 0) {
            *lengthMinP = atoi(argv[i + 1]);
            if ((strlen(argv[i + 1]) > 1) && (*lengthMinP == 0)) {
                fprintf(stderr, USAGE_ERROR);
                exit(USAGE_ERROR_CODE);
            }
            if (!(isdigit(argv[i + 1][0]))) {
                fprintf(stderr, USAGE_ERROR);
                exit(USAGE_ERROR_CODE);
            }
        } else {
            fprintf(stderr, USAGE_ERROR);
            exit(USAGE_ERROR_CODE);
        }
    }
}

/* update_score()
 * Updates the score based on the length of the input word
 * score: A pointer to the current score
 * input: The user input
 * setLength: the length of the set used to determine eligibility for bonus
 */
void update_score(int* score, char* input, int setLength)
{
    int length = strlen(input);
    *score += length;
    if (length == setLength) {
        *score += MAX_LEN_BONUS;
    }
}

/* final_score()
 * Prints to the terminal the final score with appropriate terminal message
 */
void final_score(int score)
{
    printf("Game over. You scored %d\n", score);
}

/* no_words_end()
 * Checks if the user had an alternate game ending where they entered no words
 */
void no_words_end(int score)
{
    if (score > 0) {
        final_score(score);
    } else {
        printf("No words!\n");
        exit(NO_WORDS_ERROR_CODE);
    }
}

/**
 * Used to free the data structure that stored the sorted dictionary words
 * at the end of the game.
 */
void free_words_array(char** words, int numLines)
{
    // free each char* and then the char** itself
    for (int i = 0; i < numLines; i++) {
        free(words[i]);
    }
    free(words);
}

/* free_input_words()
 * Frees the data structure that stored entered user inputs at the end of the
 * game
 *
 * Depending on how the game ends, extra variables for to free the dictionary
 * is also processed to be invoked.
 */
void free_input_words(
        char** inputWords, int numWords, char** words, int numLines)
{
    for (int i = 0; i < numWords; i++) {
        free(inputWords[i]);
    }
    free(inputWords);
    free_words_array(words, numLines);
}

/* end_game()
 * When the user quits this function calculates all possible words that can be
 * formed from the letterset based on the dictionary and outputs the maximum
 * possible score.
 *
 * words: A data structure containing words from the dictionary file
 * numLines: An index variable used to keep track of the words
 * lengthMin: Minimum length for a world
 * setLength: Length of the provided letter set (max length)
 * LetterSetCount: Frequencies of each letter in the letter set.
 */
void end_game(char** words, int numLines, int lengthMin, int setLength,
        const int* letterSetCount)
{
    // Essentially emulates playing the game but with the dictionary instead
    // of user input
    int maxScore = 0;
    for (int i = 0; i < numLines; i++) {
        char* dictInput = words[i];
        int dictLetterCount[ALPHABET_SIZE] = {0};
        for (size_t j = 0; j < strlen(dictInput); j++) {
            dictLetterCount[toupper(dictInput[j]) - 'A']++;
        }
        size_t match = 0;
        for (int k = 0; k < ALPHABET_SIZE; k++) {

            // if the frequency count for the current letter is less than the
            //  frequency count for that letter in the letterset then that
            //  letter can be formed from the letter set
            if (dictLetterCount[k] <= letterSetCount[k]) {
                match += 1;
            }
        }
        // if the number of elibile and matching letter frequencies match the
        // length of the alphabet...
        if (match == ALPHABET_SIZE && (int)strlen(dictInput) >= lengthMin) {
            update_score(&maxScore, dictInput, setLength);
            printf("%s\n", dictInput);
        }
    }
    printf("The maximum possible score is %d\n", maxScore);
}

int main(int argc, char* argv[])
{
    char* letters = NULL;
    char* dictPath = "/local/courses/csse2310/etc/words";
    int lengthMin = DEFAULT_MIN_LENGTH;
    int argC = argc - 1; // Correct number of args excluding prog-name
    arg_check(argC, argv); // Verify validity of arguments
    parse_args(argC, argv, &letters, &dictPath, &lengthMin);
    int setLength = 0; // Setup/verify letter set
    letters = letter_set_check(letters, &setLength, lengthMin);
    FILE* dictFile = file_check(dictPath);
    int numLines = 0; // Setup the word array
    char** words = add_to_array(&setLength, dictFile, &numLines);
    int letterSetCount[ALPHABET_SIZE] = {0}; // Count frequency of each letter
    for (size_t i = 0; i < strlen(letters); i++) {
        letterSetCount[toupper(letters[i]) - 'A']++;
    }
    printf("Welcome to UQunscramble!\n");
    printf("Enter words of length %d to %d made from the letters \"%s\"\n",
            lengthMin, setLength, letters);
    char* input = read_line(stdin);
    int arraySize = INIT_BUFFER;
    char** inputWords = malloc(arraySize * sizeof(char*));
    int numWords = 0;
    int score = 0;
    while (input != NULL) {
        if (strcmp(input, "Q") == 0) {
            end_game(words, numLines, lengthMin, setLength, letterSetCount);
            free(input);
            free_input_words(inputWords, numWords, words, numLines);
            no_words_end(score);
            return 0;
        }
        if (verify_input(input, lengthMin, &setLength, letterSetCount,
                    inputWords, numWords, words, &numLines)
                == 0) {
            if (numWords == arraySize) {
                arraySize *= 2;
                inputWords = realloc(inputWords, arraySize * sizeof(char*));
            }
            inputWords[numWords++] = input;
            update_score(&score, input, setLength);
            printf("Good! Your score so far is %d\n", score);
        }
        input = read_line(stdin);
    }
    free_input_words(inputWords, numWords, words, numLines);
    no_words_end(score);
    return 0;
}
