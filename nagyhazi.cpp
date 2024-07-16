#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct car // Az autók struktúrája.
{
    char brand[50];
    char plate[20];
    char pofc[50];
    struct car *next;
} car;

typedef struct fault // A problémák/hibák struktúrája.
{
    char name[50];
    int price;
    struct fault *next;
} fault;

typedef struct service // A szervízek struktúrája.
{
    char name[50];
    int capacity;
    fault *repair_list;
    car *car_list;
    struct service *next;
} service;

// Megkapja az első hiba címét, a költségét és a nevét, majd listát csinál belőlük.
// Ha kész a lista hozzáfűzi az adott szervízhez.
void create_fault_list(fault *head, int price, char *buffer)
{
    fault *current = head;
    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = (fault*)malloc(sizeof(fault)); // Memóriát foglalunk az új problémaelemnek.
    if (current->next == NULL)
    {
        printf("Nem sikerült memóriát foglalni!\n"); // Ellenőrizzük a memóriafoglalás sikerességét.
        return;
    }
    current->next->price = price;
    buffer[strlen(buffer) - 1] = '\0';
    strcpy(current->next->name, buffer);
    current->next->next = NULL;
}

// Megkapja az elsőnek beolvasott szervíz címét és a következő szervíz címét
//Összeláncolja őket.
void create_service_list(service *head, service *new_service)
{
    service *p = NULL;
    for (p = head; p->next != NULL; p = p->next)
        ;
    p->next = new_service;
    p->next->next = NULL;
}

// Beolvassa az egyes szervizek után lévő problémalistát.
// Pontosabban a nevét és az árát.

fault *scan_fault_list(FILE *f)
{
    int status = 0; // Ellenőrizzük a beolvasás sikerességét.
    char buffer[50];
    int price;
    fault *fault_list = (fault *)malloc(sizeof(fault)); // Memóriát foglalunk az elsőként beolvasott problémának.
    if (fault_list == NULL)
    {
        printf("Nem sikerült memóriát foglalni!\n");
        return NULL;
    }
    fault_list->next = NULL;
    status = fscanf(f, "%[^-/] - %d\n", buffer, &(fault_list->price)); // Addig olvasunk amíg a következő szervíz nem jönne,
    if (status == 0)                                                   // vagy amíg a "-" karakterrel nem találkozunk.
    {
        printf("Nem sikerült a beolvasás!\n"); // A nevet és az árat egyszerre olvassuk be kihasználva azt,
        return 0;                              // hogy "-" választja el őket egymástól.
    }
    buffer[strlen(buffer) - 1] = '\0';
    strcpy(fault_list->name, buffer);

    while (fscanf(f, "%[^-/] - %d\n", buffer, &price) == 2)
    {
        create_fault_list(fault_list, price, buffer); // Elkezdjük a problémalistát beolvasni és összeláncolni.
    }
    status = fscanf(f, "%s\n//////", buffer); // A szervízeket ez a karakterkombináció választja el egymástól.
    if (status == 0)                          // Mielőtt új szervízeket olvasunk, ezt be kell olvasni.
    {
        printf("Nem sikerült a beolvasás!\n");
        return 0;
    }
    return fault_list; // Visszaadjuk a kész problémalista kezdőcímét.
}

// Az egyik főfüggvény, mely a szervízeket olvassa be és a kész problémalistákat egyenként hozzájuk kapcsolja.
service *read_service()
{
    char buffer[50];
    int capacity, status1 = 0, status2 = 0; // Status1 a beolvasás, status2 a fájlbezárás sikerességét ellenőrzi.
    FILE *f;                                // Megnyitjuk a szervízeket tartalmazó szöveges fájlt olvasásra.
    f = fopen("szervizek.txt", "r");
    if (f == NULL)
    {
        printf("Nem sikerült megnyitni a szervizek.txt-t!\n");
        return NULL;
    }
    
    service *service_list = (service *)malloc(sizeof(service)); // Memóriát foglalunk a legelső szervíznek.
    if (service_list == NULL)
    {
        printf("Nem sikerült memóriát foglalni!\n");
        return NULL;
    }
    service_list->car_list = NULL;
    service_list->repair_list = NULL;
    service_list->next = NULL;          // Beállítjuk az "üres" szervízt reprezentáló struktúrát.
    char *check = fgets(buffer, 50, f); // Ellenőrizzük az fgets sikerességét, beolvassuk a szervíz nevét.
    if (check == NULL)
    {
        printf("Nem sikerült a beolvasás!\n");
        return NULL;
    }
    buffer[strlen(buffer) - 1] = '\0';                      // Kivesszük az fgets által beolvasott enter karaktert a bufferból.
    status1 = fscanf(f, "%d\n", &(service_list->capacity)); // Beolvassuk a szervíz kapacitását.
    if (status1 == 0)
    {
        printf("Nem sikerült a beolvasás!\n");
        return 0;
    }
    strcpy(service_list->name, buffer);     // A buffer tartalma a szervíz neve lesz.
    fault *fault_list = scan_fault_list(f); //Új problémaelemet hozunk létre.
    service_list->repair_list = fault_list;
    while (fgets(buffer, 50, f) != NULL && fscanf(f, "%d\n", &(capacity)) == 1) // Addig olvassuk a szervízek adatait, amíg tudjuk.
    {
        service *new_service = (service *)malloc(sizeof(service)); // A soron következő szervízeket olvassuk,
        if (new_service == NULL)                                   // memóriát foglalunk.
        {
            printf("Nem sikerült memóriát foglalni!\n");
            return NULL;
        }
        
        new_service->car_list = NULL;
        buffer[strlen(buffer) - 1] = '\0'; // Entert kivesszük a bufferből.
        strcpy(new_service->name, buffer); // A szervíz nevét a buffer tartalma adja majd.
        new_service->capacity = capacity;
        fault *fault_list = scan_fault_list(f);
        new_service->repair_list = fault_list; // A kész problémalistát hozzáláncoljuk a szervízhez.
        new_service->next = NULL;
        create_service_list(service_list, new_service); // Meghívjuk a szervízösszeláncoló függvényt.
    }
    status2 = fclose(f); // Bezárjuk a szervízeket tartalmazó fájlt.
    if (status2 != 0)
    {
        printf("A szervizek.txt-t nem sikerült bezárni!\n");
        return NULL;
    }
    return service_list; // Visszaadjuk a már problémalistákkal kiegészült szervízlista kezdőcímét.
}

car *create_car_list(car *head, car *new_car) // Megkapja a legelsőként beolvasott autó címét,
{                                             // illetve az újonnan beolvasottét is.
    car *p = NULL;                            // Ezek után összeláncolja őket.
    if (head == NULL)
    {
        head = new_car;
        head->next = NULL;
    }
    else
    {
        for (p = head; p->next != NULL; p = p->next) // Lista végéhez láncolunk mindig.
            ;
        p->next = new_car;
        p->next->next = NULL;
    }
    return head; // Visszaadjuk az autók listájának kezdőcímét.
}
// A főalgoritmus, mely megkapja a kész szervízlistát, ami egy fésűs lista.
// Megkeresi a legolcsóbb olyan nem teli szervízt, melyben van olyan szolgáltatás,
// amit az autó kíván.
void find_cheapest(service *list, car *current_car)
{
    int minprice = 0;               // A minimális javítási költség.
    service *p = NULL, *tmp = NULL; // P a szervízeken ugrál végig, tmp pedig megjegyzi az eddigi talált legolcsóbbat.
    fault *l = NULL;                // L a szervízen belüli problémalistán ugrál végig.
    for (p = list; p != NULL; p = p->next)
    {
        if (p->capacity == 0) // Ellenőrizzük, hogy egyáltalán van-e még hely.
            continue;
        for (l = p->repair_list; l != NULL; l = l->next) // Minden szervíznél megállunk és "benézünk".
        {
            if (strcmp(l->name, current_car->pofc) == 0)
            {
                if (minprice == 0)
                {
                    minprice = l->price; // Kinevezzük az elsőként talált árat a legjobbnak, később változhat.
                    tmp = p;             // Megjegyezzük az eddigi legjobb szervízt.
                    break;
                }
                else if (minprice > l->price)
                {
                    minprice = l->price;
                    tmp = p;
                    break;
                }
            }
        }
    }

    if (minprice == 0) // Ha a minprice 0 maradt az azt jelenti, hogy nem találtunk megfelelő szolgáltatást,
    {                  // vagy minden szervíz tele volt.
        printf("////////////////////////////////\n");
        printf("Az autó felvétele sikertelen!\n");
        printf("Márka: %s\nRendszám: %s\n", current_car->brand, current_car->plate);
        printf("Nem található ilyen szolgáltatás a még be nem telt szervizek között.\n");
        printf("////////////////////////////////\n\n");
        free(current_car); // Ha nem tudtuk elhelyezni az autót, akkor felszabadítjuk, hiszen nem kerül be a listába.
    }
    else
    {
        printf("////////////////////////////////\n");
        printf("Az autó felvétele sikeres!\n");
        printf("Márka: %s\nRendszám: %s\nSzerviz: %s\nHiba: %s\nJavítási költség: %d Ft\n",
               current_car->brand, current_car->plate,
               tmp->name, current_car->pofc, minprice);
        printf("////////////////////////////////\n\n");
        tmp->capacity = tmp->capacity - 1;                           // Sikerült elhelyezni az autót, így a kapacitást csökkentenünk kell.
        tmp->car_list = create_car_list(tmp->car_list, current_car); // Hozzáfűzzük az autót az adott szervízhez.
    }
}
// Beolvassa az autókat egyenként, minden adatával együtt, majd átadjuk őket a find_cheapest függvénynek.
// A program minden autót sorban és egyenlént olvas be, valamint helyez el.
void read_car(service *list)
{
    FILE *g; // Megnyitjuk az autókat tartalmazó txt fájlt.
    g = fopen("autok.txt", "r");
    if (g == NULL) // Ellenőrizzük, hogy a fájlt sikerült-e megnyitni.
    {
        printf("Nem sikerült megnyitni az autok.txt-t!\n");
        return;
    }
    int status1 = 0, status2 = 0; // Status1 az olvasás, status2 a bezárás sikerességét ellenőrzi.
    char buffer1[50];
    char buffer2[20];
    char buffer3[50];

    while ((fgets(buffer1, 50, g) != NULL) && (fgets(buffer2, 20, g) != NULL) && (fgets(buffer3, 50, g) != NULL))
    {                                              // Egyszerre olvassuk az autó adatait (márka, rendszám, probléma).
        car *new_car = (car *)malloc(sizeof(car)); // Memóriát foglalunk az új autónak.
        if (new_car == NULL)
        {
            printf("Nem sikerült memóriát foglalni!\n");
            return;
        }
        new_car->next = NULL;
        buffer1[strlen(buffer1) - 1] = '\0'; // Mivel az fgets beolvassa az enter karaktert is,
        buffer2[strlen(buffer2) - 1] = '\0'; // így ezt kivesszük a bufferből.
        buffer3[strlen(buffer3) - 1] = '\0';
        strcpy(new_car->brand, buffer1);
        strcpy(new_car->plate, buffer2);
        strcpy(new_car->pofc, buffer3);
        status1 = fscanf(g, "%c", buffer1);
        if (status1 == 0)
        {
            printf("Nem sikerült a beolvasás!\n");
            return;
        }
        find_cheapest(list, new_car);
    }
    status2 = fclose(g);
    if (status2 != 0)
    {
        printf("Az autok.txt-t nem sikerült bezárni!\n");
        return;
    }
}

// Az autó márkáját nagybetűsre alakítja, illetve a rendszámot is, ha szükséges.
// A problémát pedig kisbetűssé ha szükséges.
void char_changer(char *char1, char *char2, char *char3)
{
    int i = 0;
    char1[strlen(char1) - 1] = '\0';
    char2[strlen(char2) - 1] = '\0';
    char3[strlen(char3) - 1] = '\0';

    while (char1[i] != '\0')
    {
        if (char1[i] >= 'a' && char1[i] <= 'z')
            char1[i] = char1[i] - 32; // Kisbetűs volt, nagybetűs lesz.
        i++;
    }
    i = 0;
    while (char2[i] != '-') // A számokkal nem foglalkozunk már.
    {
        if (char2[i] >= 'a' && char2[i] <= 'z')
            char2[i] = char2[i] - 32; // Kisbetűs volt, nagybetűs lesz.
        i++;
    }
    i = 0;
    while (char3[i] != '\0')
    {
        if (char3[i] >= 'A' && char3[i] <= 'Z')
            char3[i] = char3[i] + 32; // Nagybetűs volt, kisbetűs lesz.
        i++;
    }
}

// Az autok.txt fájlt tudjuk bővíteni:
// megkapja az autó adatait és beírja az autok.txt-be.
// A program akkor kezdi el végezni a fő feladatát, amikor befejezzük a hozzácsatolást.
void attach_car(char *brand, char *plate, char *problem)
{
    int status = 0;
    FILE *f; // Megnyitjuk az autókat tartalmazó fájlt bővítésre.
    f = fopen("autok.txt", "a");
    if (f == NULL)
    {
        printf("Nem sikerült megnyitni az autok.txt-t!\n");
        return;
    }
    char_changer(brand, plate, problem);

    fprintf(f, "%s\n", brand);
    fprintf(f, "%s\n", plate);
    fprintf(f, "%s\n\n", problem);
    status = fclose(f);
    if (status != 0)
    {
        printf("Az autok.txt-t nem sikerült bezárni!\n");
        return;
    }
}
// Kitörli az autókat tartalmazó listát.
car *dispose_carlist(car *head)
{
    if (head == NULL)
        return NULL;
    car *p = head;
    car *prev = NULL;
    while (p->next != NULL) // Elmegyünk a lista végére.
    {
        prev = p;
        p = p->next;
    }
    if (prev != NULL)
        prev->next = NULL;

    if (p == head)
        head = NULL;

    free(p); // Felszabadítjuk a lista végén található elemet.
    p = NULL;

    return head; // Visszaadjuk az új lista kezdőcímét.
}
// Kitörli a hibákat tartalmazó listát.
fault *dispose_faultlist(fault *head)
{
    if (head == NULL)
        return NULL;
    fault *p = head;
    fault *prev = NULL;
    while (p->next != NULL) // Elmegyünk a lista végére.
    {
        prev = p;
        p = p->next;
    }
    if (prev != NULL)
        prev->next = NULL;

    if (p == head)
        head = NULL;

    free(p); // Felszabadítjuk a lista végén található elemet.
    p = NULL;
    return head; // Visszaadjuk az új lista kezdőcímét.
}
// Kitörli a szervízek listáját, melynek autó-, illetve hibalistája már üres.
service *dispose_list(service *list)
{
    service *sp = NULL;
    car *cp = NULL;
    fault *fp = NULL;
    for (sp = list; sp != NULL; sp = sp->next) // Végigmegyünk minden szervízen.
    {
        if (sp->car_list != NULL)
        {
            cp = sp->car_list;
            while (cp != NULL)
            {
                cp = dispose_carlist(cp); // Felszabadítjuk az autók listáját minden szervíznél.
            }
        }
        if (sp->repair_list != NULL)
        {
            fp = sp->repair_list;
            while (fp != NULL)
            {
                fp = dispose_faultlist(fp); // Felszabadítjuk a hibák listáját minden szervíznél.
            }
        }
    }
    while (list != NULL) // Végül a szervízeket szabadítjuk fel.
    {
        if (list == NULL)
            return NULL;
        service *p = list;
        service *prev = NULL;
        while (p->next != NULL) // Elmegyünk a lista végére.
        {
            prev = p;
            p = p->next;
        }
        if (prev != NULL)
            prev->next = NULL;

        if (p == list)
            list = NULL;

        free(p); // Felszabadítjuk a lista végén található elemet;
    }
    return list; // Visszaadjuk az új lista kezdőcímét.
}

// A main függvény a felhasználói interfacet valósítja meg, valamint meghívja a főfüggvényeket.
int main()
{
    char buffer1[50];
    char buffer2[20];
    char buffer3[50];
    char choice[5];
    char *p, tmp;
    printf("Kíván új autót felvenni?\n\n");
    printf("IGEN <-> NEM\n");
    scanf("%s", choice);
    scanf("%c", &tmp);               // Döntési lehetőség.
    if (strcmp(choice, "IGEN") == 0) // Ha az igent választottuk, akkor új autót veszünk fel.
    {
        printf("Kérjük adja meg az új autó adatait:\n");
        while (1)
        {
            printf("Márka: ");
            p = fgets(buffer1, 50, stdin); // Standard inputról olvassuk az adatokat.
            if (p == NULL)
            {
                printf("Nem sikerült a beolvasás!\n");
                return 0;
            }
            printf("Rendszám: ");
            p = fgets(buffer2, 10, stdin);
            if (p == NULL)
            {
                printf("Nem sikerült a beolvasás!\n");
                return 0;
            }
            printf("Probléma: ");
            p = fgets(buffer3, 50, stdin);
            if (p == NULL)
            {
                printf("Nem sikerült a beolvasás!\n");
                return 0;
            }

            attach_car(buffer1, buffer2, buffer3); // Beírja az autó adatait az autókat tartalmazó fájlba.

            printf("\nKívánja folytatni?\n");
            printf("IGEN <-> NEM\n");
            scanf("%s", choice); // Ismét választhatunk, hogy akarunk-e új autót felvenni.
            if (strcmp(choice, "NEM") == 0)
            {
                break; // Ha nem, akkor kilépünk a ciklusból.
            }
            scanf("%c", &tmp); // Beolvassuk standard inputon maradt entert.
        }
    }
    service *servicelist = read_service(); // Létrehozzuk a szervíz listáját, ahol a szervízek még üresek.
    read_car(servicelist);
    servicelist = dispose_list(servicelist); // Kitörli a teljes listát.
    return 0;
}