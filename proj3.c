/**
 * proj3.c - Jednoducha shlukova analyza
 * 
 * Author: Jan Svabik, xsvabi00
 * 1BIT, 2018/19
 * 
 * Created: 2018-12-02
 * Last update: 2018-12-08
 */

/**
 * Kostra programu pro 3. projekt IZP 2018/19
 *
 * Jednoducha shlukova analyza: 2D nejblizsi soused.
 * Single linkage
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h> // sqrtf
#include <limits.h> // INT_MAX

#define OBJECT_LINE_MAX 1024

/*****************************************************************
 * Ladici makra. Vypnout jejich efekt lze definici makra
 * NDEBUG, napr.:
 *   a) pri prekladu argumentem prekladaci -DNDEBUG
 *   b) v souboru (na radek pred #include <assert.h>
 *      #define NDEBUG
 */
#ifdef NDEBUG
#define debug(s)
#define dfmt(s, ...)
#define dint(i)
#define dfloat(f)
#else

// vypise ladici retezec
#define debug(s) printf("- %s\n", s)

// vypise formatovany ladici vystup - pouziti podobne jako printf
#define dfmt(s, ...) printf(" - "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

// vypise ladici informaci o promenne - pouziti dint(identifikator_promenne)
#define dint(i) printf(" - " __FILE__ ":%u: " #i " = %d\n", __LINE__, i)

// vypise ladici informaci o promenne typu float - pouziti
// dfloat(identifikator_promenne)
#define dfloat(f) printf(" - " __FILE__ ":%u: " #f " = %g\n", __LINE__, f)

#endif

/*****************************************************************
 * Deklarace potrebnych datovych typu:
 *
 * TYTO DEKLARACE NEMENTE
 *
 *   struct obj_t - struktura objektu: identifikator a souradnice
 *   struct cluster_t - shluk objektu:
 *      pocet objektu ve shluku,
 *      kapacita shluku (pocet objektu, pro ktere je rezervovano
 *          misto v poli),
 *      ukazatel na pole shluku.
 */

struct obj_t {
    int id;
    float x;
    float y;
};

struct cluster_t {
    int size;
    int capacity;
    struct obj_t *obj;
};

/*****************************************************************
 * Deklarace potrebnych funkci.
 *
 * PROTOTYPY FUNKCI NEMENTE
 *
 * IMPLEMENTUJTE POUZE FUNKCE NA MISTECH OZNACENYCH 'TODO'
 *
 */

/*
 Inicializace shluku 'c'. Alokuje pamet pro cap objektu (kapacitu).
 Ukazatel NULL u pole objektu znamena kapacitu 0.
*/
void init_cluster(struct cluster_t *c, int cap)
{
    assert(c != NULL);
    assert(cap >= 0);

    c->size = 0;
    c->capacity = cap;
    if (cap == 0)
        c->obj = NULL;
    else
        c->obj = malloc(sizeof(struct obj_t) * cap);
}

/*
 Odstraneni vsech objektu shluku a inicializace na prazdny shluk.
 */
void clear_cluster(struct cluster_t *c)
{
    free(c->obj);
    init_cluster(c, 0);
}

/// Chunk of cluster objects. Value recommended for reallocation.
const int CLUSTER_CHUNK = 10;

/*
 Zmena kapacity shluku 'c' na kapacitu 'new_cap'.
 */
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap)
{
    // TUTO FUNKCI NEMENTE
    assert(c);
    assert(c->capacity >= 0);
    assert(new_cap >= 0);

    if (c->capacity >= new_cap)
        return c;

    size_t size = sizeof(struct obj_t) * new_cap;

    void *arr = realloc(c->obj, size);
    if (arr == NULL)
        return NULL;

    c->obj = (struct obj_t*)arr;
    c->capacity = new_cap;
    return c;
}

/*
 Prida objekt 'obj' na konec shluku 'c'. Rozsiri shluk, pokud se do nej objekt
 nevejde.
 */
void append_cluster(struct cluster_t *c, struct obj_t obj)
{
    if (c->size >= c->capacity)
        if (resize_cluster(c, c->capacity + CLUSTER_CHUNK) == NULL)
            return;

    c->obj[c->size] = obj;
    c->size++;
}

/*
 Seradi objekty ve shluku 'c' vzestupne podle jejich identifikacniho cisla.
 */
void sort_cluster(struct cluster_t *c);

/*
 Do shluku 'c1' prida objekty 'c2'. Shluk 'c1' bude v pripade nutnosti rozsiren.
 Objekty ve shluku 'c1' budou serazeny vzestupne podle identifikacniho cisla.
 Shluk 'c2' bude nezmenen.
 */
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c2 != NULL);

    for (int i = 0; i < c2->size; i++)
        append_cluster(c1, c2->obj[i]);

    sort_cluster(c1);    
}

/**********************************************************************/
/* Prace s polem shluku */

/*
 Odstrani shluk z pole shluku 'carr'. Pole shluku obsahuje 'narr' polozek
 (shluku). Shluk pro odstraneni se nachazi na indexu 'idx'. Funkce vraci novy
 pocet shluku v poli.
*/
int remove_cluster(struct cluster_t *carr, int narr, int idx)
{
    assert(idx < narr);
    assert(narr > 0);

    narr -= 1;
    clear_cluster(&carr[idx]);

    for (int i = idx; i < narr; i++)
        carr[i] = carr[i+1];

    return narr;
}

/*
 Pocita Euklidovskou vzdalenost mezi dvema objekty.
 */
float obj_distance(struct obj_t *o1, struct obj_t *o2)
{
    assert(o1 != NULL);
    assert(o2 != NULL);

    return sqrtf(pow(o1->x - o2->x, 2) + pow(o1->y - o2->y, 2));
}

/*
 Pocita vzdalenost dvou shluku.
*/
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c1->size > 0);
    assert(c2 != NULL);
    assert(c2->size > 0);

    float minDistance = __FLT_MAX__;
    
    for (int i = 0; i < c1->size; i++) {
        for (int j = 0; j < c2->size; j++) {
            float distance = obj_distance(&(c1->obj[i]), &(c2->obj[j]));
            if (distance < minDistance)
                minDistance = distance;
        }
    }
    
    return minDistance;
}

/*
 Funkce najde dva nejblizsi shluky. V poli shluku 'carr' o velikosti 'narr'
 hleda dva nejblizsi shluky. Nalezene shluky identifikuje jejich indexy v poli
 'carr'. Funkce nalezene shluky (indexy do pole 'carr') uklada do pameti na
 adresu 'c1' resp. 'c2'.
*/
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2)
{
    assert(narr > 0);

    float minDistance = __FLT_MAX__;
    for (int i = 0; i < narr; i++) {
        for (int j = i + 1; j < narr; j++) {
            float distance = cluster_distance(&(carr[i]), &(carr[j]));
            if (distance < minDistance) {
                minDistance = distance;
                *c1 = i;
                *c2 = j;
            }
        }
    }
}

// pomocna funkce pro razeni shluku
static int obj_sort_compar(const void *a, const void *b)
{
    // TUTO FUNKCI NEMENTE
    const struct obj_t *o1 = (const struct obj_t *)a;
    const struct obj_t *o2 = (const struct obj_t *)b;
    if (o1->id < o2->id) return -1;
    if (o1->id > o2->id) return 1;
    return 0;
}

/*
 Razeni objektu ve shluku vzestupne podle jejich identifikatoru.
*/
void sort_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    qsort(c->obj, c->size, sizeof(struct obj_t), &obj_sort_compar);
}

/*
 Tisk shluku 'c' na stdout.
*/
void print_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    for (int i = 0; i < c->size; i++)
    {
        if (i) putchar(' ');
        printf("%d[%g,%g]", c->obj[i].id, c->obj[i].x, c->obj[i].y);
    }
    putchar('\n');
}

// check that integer is in array of integers
int in_int_array(int* arr, int arr_length, int id) {
    for (int i = 0; i < arr_length; i++)
        if (arr[i] == id) {
            return 1;
            break;
        }
    
    return 0;
}

// deallocate clusters in array and its objects
void dealloc(struct cluster_t *clusters, int clusterCount) {
    for (int i = 0; i < clusterCount; i++)
        free(clusters[i].obj);
    free(clusters);
}

// function for deallocating clusters, ids and close opened file if there is some error during loading clusters
int deallocBecauseOfError(struct cluster_t *clusters, int clusterCount, int* ids, FILE* objectFile) {
    dealloc(clusters, clusterCount);
    free(ids);
    fclose(objectFile);

    return -1;
}

/*
 Ze souboru 'filename' nacte objekty. Pro kazdy objekt vytvori shluk a ulozi
 jej do pole shluku. Alokuje prostor pro pole vsech shluku a ukazatel na prvni
 polozku pole (ukalazatel na prvni shluk v alokovanem poli) ulozi do pameti,
 kam se odkazuje parametr 'arr'. Funkce vraci pocet nactenych objektu (shluku).
 V pripade nejake chyby uklada do pameti, kam se odkazuje 'arr', hodnotu NULL.
*/
int load_clusters(char *filename, struct cluster_t **arr)
{
    assert(arr != NULL);

    // open the file
    FILE* objectFile = fopen(filename, "r");
    if (objectFile == NULL) {
        printf("File \"%s\" cannot be opened. Maybe it doesn't exist.\n", filename);
        return -1;
    }

    // prepare variables for loop and buffer
    char line[OBJECT_LINE_MAX];

    // get first line of file (number of objects)
    fgets(line, OBJECT_LINE_MAX, objectFile);
    
    // get number of objects
    int clusterCount = 0;
    sscanf(line, "count=%d", &clusterCount);

    // allocate memory for ids and clusters
    int *ids = malloc(clusterCount * sizeof(int));
    *arr = malloc(clusterCount * sizeof(struct cluster_t));
    if (*arr == NULL || ids == NULL) {
        printf("Memory cannot be allocated.\n");
        return deallocBecauseOfError(*arr, 0, ids, objectFile);
    }

    int loaded = 0;
    while (loaded < clusterCount) {
        // load new line, if there is not any new line to load, stop the loop
        if (fgets(line, OBJECT_LINE_MAX, objectFile) == NULL)
            break;

        // parse line
        int id;
        float x, y;
        sscanf(line, "%d %f %f", &id, &x, &y);

        // check that the id is really unique and the coordinates are in the required range
        int idCheck = in_int_array(ids, loaded, id);
        if (idCheck || x < 0 || x > 1000 || y < 0 || y > 1000) {
            if (idCheck)
                printf("The ID %d was already used. ID must be unique.\n", id);
            else
                printf("Coordinates out of range. 0 <= X/Y <= 1000. X = %f, Y = %f.\n", x, y);

            return deallocBecauseOfError(*arr, 0, ids, objectFile);
        }

        // add id to array
        ids[loaded] = id;

        // create object with parsed data
        struct obj_t obj;
        obj.id = id;
        obj.x = x;
        obj.y = y;

        // create new cluster and push the object into
        init_cluster(&(*arr)[loaded], 1);
        if ((*arr)[loaded].obj == NULL) {
            printf("Error when trying to allocate memory for new cluster.\n");
            return deallocBecauseOfError(*arr, 0, ids, objectFile);
        }

        // append successfully initialized cluster to the array
        append_cluster(&(*arr)[loaded], obj);

        loaded++;
    }

    free(ids);
    fclose(objectFile);

    if (loaded < clusterCount) {
        printf("Missing %d objects in file \"%s\". count=%d, given=%d.\n", clusterCount-loaded, filename, clusterCount, loaded);
        dealloc(*arr, loaded);
        return -1;
    }

    return loaded;
}

/*
 Tisk pole shluku. Parametr 'carr' je ukazatel na prvni polozku (shluk).
 Tiskne se prvnich 'narr' shluku.
*/
void print_clusters(struct cluster_t *carr, int narr)
{
    printf("Clusters:\n");
    for (int i = 0; i < narr; i++)
    {
        printf("cluster %d: ", i);
        print_cluster(&carr[i]);
    }
}

int checkArguments(int argc, char* argv[]) {
    // check number of arguments
    if (argc < 2 || argc > 3) {
        printf("Wrong arguments, the correct syntax is:\n./proj3 soubor_s_objekty [pocet_clusteru]\n");
        return 0;
    }

    char* endPtr;
    int requiredClusters = strtol(argv[2], &endPtr, 10);

    // check that the number of required clusters is number
    if (*endPtr != '\0') {
        printf("The last argument should be an integer. Your input: %s.\n", argv[2]);
        return 0;
    }

    // check that the number of required clusters is greather than 0
    if (argc == 3 && requiredClusters < 1) {
        printf("The number of required clusters must be greather that 0.\n");
        return 0;
    }

    return 1;
}

int main(int argc, char *argv[])
{
    // check program arguments
    if (!checkArguments(argc, argv))
        return EXIT_SUCCESS;

    // prepare variable for array of clusters
    struct cluster_t *clusters;

    // load the number of required clusters and prepare counter for clusters
    int clusterCount = load_clusters(argv[1], &clusters);    
    int requiredClusters = 1;

    // check that there was not any error
    if (clusterCount == -1)
        return EXIT_FAILURE;

    // if optional argument was given (number of requried clusters), use it
    if (argc == 3)
        requiredClusters = strtol(argv[2], NULL, 10);

    // if the user wants more clusters than there are objects, create only the maximum
    if (requiredClusters > clusterCount)
        requiredClusters = clusterCount;

    // do the clustering
    int c1, c2;
    for (unsigned int i = 0; clusterCount != requiredClusters; i++) {
        find_neighbours(clusters, clusterCount, &c1, &c2);
        merge_clusters(&(clusters[c1]), &(clusters[c2]));
        clusterCount = remove_cluster(clusters, clusterCount, c2);
    }

    // print clusters
    print_clusters(clusters, clusterCount);

    // deallocate the memory
    dealloc(clusters, clusterCount);

    return EXIT_SUCCESS;
}