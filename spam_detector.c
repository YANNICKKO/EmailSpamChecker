#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

int strstrcase(char *str1, char *str2)
{
    char *dup = strdup(str1);
    char *temp;
    int nr_repetari = 0;

    for (unsigned int i = 0; i < strlen(dup); i++)
        if (dup[i] >= 'A' && dup[i] <= 'Z')
        {
            dup[i] -= 'A' - 'a';
        }
    temp = strstr(dup, str2);

    while (temp != NULL)
    {
        nr_repetari++;
        temp = strstr(temp + 1, str2);
    }

    return nr_repetari;
}

int words_line(char *line)
{
    int contor = 0;
    int ok = 1;
    for (unsigned int i = 0; i < strlen(line) - 1; i++)
        if ((line[i] >= 'a' && line[i] <= 'z') || (line[i] >= 'A' && line[i] <= 'Z'))
        {
            if (ok == 1)
            {
                contor++;
            }
            ok = 0;
        }
        else
            ok = 1;

    if ((line[strlen(line)] >= 'a' && line[strlen(line)] <= 'z') || (line[strlen(line)] >= 'A' && line[strlen(line)] <= 'Z'))
        if (ok == 0)
            contor++;

    return contor;
}

float averageSize(int *len, int nr_email)
{
    float avgSize = 0;
    for (int i = 0; i < nr_email; i++)
        avgSize += len[i];

    avgSize /= nr_email;
    return avgSize;
}

float keywordsScore(int **count, float avgSize, int len, int nr_email, int nr_keywords)
{
    float sum = 0;
    for (int i = 0; i < nr_keywords; i++)
    {
        sum += count[i][nr_email];
    }
    sum = sum * avgSize / len;

    return sum;
}

int hasCaps(char *line)
{
    int contor = 0;
    int ok = 1;
    for (unsigned int i = 0; i < strlen(line) - 1; i++)
    {
        if ((line[i] >= 'A' && line[i] <= 'Z'))
        {
            ok = 0;
        }
        else if (line[i] < 'a' || line[i] > 'z')
        {
            if (ok == 0)
                contor++;
            ok = 1;
        }
        else
        {
            ok = 1;
        }
    }

    if ((line[strlen(line)] >= 'A' && line[strlen(line)] <= 'Z'))
        if (ok == 0)
            contor++;

    return contor;
}

int main()
{
    int nr_email = 0, nr_email_curent;
    DIR *emails = opendir("emails");
    struct dirent *citire;
    FILE *email, *words;
    char line[1000], **keywords, nr_cuvinte_str[100], **spammers_name, nr_spam_str[100];
    int *nr_aparitii, **stdev, nr_cuvinte, nr_spammer;
    float m_aritm, SD = 0.0;
    int len[1000] = {0}, caps[1000] = {0}, spammers[1000] = {0}, *spammers_number;

    // keywords
    //===================

    words = fopen("keywords", "r");
    fgets(nr_cuvinte_str, 100, words);
    nr_cuvinte = atoi(nr_cuvinte_str);

    keywords = (char **)malloc(nr_cuvinte * sizeof(char *));
    nr_aparitii = (int *)calloc(nr_cuvinte, sizeof(int));
    stdev = (int **)malloc(nr_cuvinte * sizeof(int *));

    for (int i = 0; i < nr_cuvinte; i++)
    {
        keywords[i] = (char *)malloc(100 * sizeof(char));
        stdev[i] = (int *)malloc(1000 * sizeof(int));
        fgets(keywords[i], 100, words);
        if (i != nr_cuvinte - 1)
            keywords[i][strlen(keywords[i]) - 1] = '\0';
    }
    fclose(words);

    //===================

    // spammers
    //===================

    words = fopen("spammers", "r");
    fgets(nr_spam_str, 100, words);
    nr_spammer = atoi(nr_spam_str);

    spammers_name = (char **)malloc(nr_spammer * sizeof(char *));
    spammers_number = (int *)malloc(nr_spammer * sizeof(int));
    for (int i = 0; i < nr_spammer; i++)
    {
        spammers_name[i] = (char *)malloc(100 * sizeof(char));
        fgets(spammers_name[i], 100, words);
        spammers_name[i][strlen(spammers_name[i]) - 1] = '\0';
        spammers_name[i] = strtok(spammers_name[i], " ");
        spammers_number[i] = atoi(strtok(NULL, " "));
    }
    fclose(words);

    //===================

    while ((citire = readdir(emails)) != NULL)
    {
        if (citire->d_type == DT_REG)
        {
            char path[] = "emails/";
            strcat(path, citire->d_name);
            email = fopen(path, "r");
            int after_body = 0;
            int nr_caps = 0;
            nr_email_curent = atoi(citire->d_name);

            while (fgets(line, 1000, email) != NULL)
            {
                if (strstr(line, "From:") != NULL)
                {
                    for (int i = 0; i < nr_spammer; i++)
                    {
                        if (strstr(line, spammers_name[i]) != NULL)
                        {
                            spammers[nr_email_curent] = spammers_number[i];
                            break;
                        }
                    }
                }

                if (strstr(line, "Body:") != NULL)
                    after_body = 1;

                if (after_body == 1)
                {
                    len[nr_email_curent] += words_line(line);
                    nr_caps += hasCaps(line);
                }

                for (int i = 0; i < nr_cuvinte && after_body == 1; i++)
                {
                    if (line[0] != '\n')
                    {
                        if (strstrcase(line, keywords[i]) != 0)
                        {
                            stdev[i][nr_email_curent] += strstrcase(line, keywords[i]);
                            nr_aparitii[i] += strstrcase(line, keywords[i]);
                        }
                    }
                }
            }
            caps[nr_email_curent] = (nr_caps > len[nr_email_curent] / 2) ? 1 : 0;
            nr_email++;
            fclose(email);
        }
    }

    for (int i = 0; i < nr_cuvinte; i++)
    {
        m_aritm = 0;
        SD = 0.0;

        m_aritm = (float)nr_aparitii[i] / nr_email;

        for (int j = 0; j < nr_email; j++)
        {
            SD += pow(stdev[i][j] - m_aritm, 2);
        }
        printf("%s %d %f\n", keywords[i], nr_aparitii[i], SD);
    }

    for (int j = 0; j < nr_email; j++)
    {
        if (((10 * keywordsScore(stdev, averageSize(len, nr_email), len[j], j, nr_cuvinte)) + (30 * caps[j]) + spammers[j]) > 35)
            printf("1\n");
        else
            printf("0\n");
    }

    closedir(emails);
    return 0;
}