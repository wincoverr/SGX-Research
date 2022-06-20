#include "Enclave_t.h"

#include "Enclave.h"
#include "Enclave_t.h"
#include <string.h>

//#include <curl/curl.h>

void print(char *str)
{
    ocall_print(str);
}

void ecall_test(void)
{
    print("test\n");
}

void ecall_connexion(char pseudo[80], char mdp[80])
{
    char buff[80];
    char username[80];
    char password[80];
    //
    strcpy(username, "mathieu");
    strcpy(password, "test");

    if (strcmp(pseudo, username) == 0 && strcmp(mdp, password) == 0)
    {
        print("Successful connection\n");
    }
    else
    {
        print("Connection failed\n");
    }

    /*
    FILE *fichier = NULL;
    fichier = fopen("users.txt", "r");

    if (fichier == NULL)
    {
        print("Erreur d'ouverture du fichier users.txt\n");
        j = 2;
    }

    fichier = fopen("users.txt", "r");
    FILE *fichier = NULL;

    if (fichier == NULL)
    {
        print("Erreur d'ouverture du fichier users.txt\n");
        j = 2;
    }

    while (fgets(buff, 80, fichier) != NULL)
    {
        sscanf(buff, "%s %s", username, password);
        if (strcmp(username, pseudo) == 0 && strcmp(password, mdp) == 0)
        {
            print("Connexion succed\n");
            j = 1;
        }
        i++;
    }

    if (j == 0)
        print("Connexion échouée\n");
*/
    print("INSIDE CONNEXION\n");
}

void ecall_meteo(char city[80])
{
    char buff[80];
    char city_name[80];
    char meteo[80];
    // api request https://api.meteo-concept.com/api/


    strcpy(city_name, "Paris");
    strcpy(meteo, "cloudy");

    if (strcmp(city, city_name) == 0)
    {
        print("Weather in Paris : cloudy\n");
    }
    else
    {
        print("Weather in Paris : sunny\n");
    }
}