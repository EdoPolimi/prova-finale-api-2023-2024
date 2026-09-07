#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>

#define HASH_BASE 7999
#define MAX_RECIPES 8000
#define MAX_SHELVES 8000
#define MAX_ORDERS 90000

struct ingredient{
    char name[21];
    int quantity;
    struct ingredient *next;
};

struct recipe{
    char name[21];
    struct ingredient *ingredients;
    int weight;
    struct recipe *next;
};

struct stock{
    int expiration;
    int quantity;
    struct stock *next;
};

struct shelf{
    char ingredient_name[21];
    int total;
    struct stock *head;
    struct stock *tail;
    struct shelf *next;
};

struct orders{
    struct order *head;
    struct order *tail;
};

struct order{
    char recipe_name[21];
    int quantity;
    int order_time;
    struct recipe *recipe;
    struct order *next;
};

int hash_function(const char *key) {
    unsigned long hash = 5381;
    int c;

    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return (int)(hash % HASH_BASE);
}

struct recipe *search_recipe(struct recipe *recipes[], char recipe_name[]){
    struct recipe *current_recipe = recipes[hash_function(recipe_name)];
    while(current_recipe != NULL && strcmp(current_recipe->name, recipe_name) != 0){
        current_recipe = current_recipe->next;
    }
    return current_recipe;
}

int insert_recipe(struct recipe *recipes[], char recipe_name[]){
    int is_existing = 0;
    //check if the recipe already exists
    if(search_recipe(recipes, recipe_name) != NULL){
        is_existing = 1;
    }
    if(is_existing == 0){
        struct recipe *new_recipe = malloc(sizeof(struct recipe));
        strcpy(new_recipe->name, recipe_name);
        new_recipe->weight = 0;
        new_recipe->ingredients = NULL;
        new_recipe->next = NULL;
        if(recipes[hash_function(recipe_name)] == NULL){
            recipes[hash_function(recipe_name)] = new_recipe;
        }
        //collision handled with linked list
        else{
            struct recipe *current = recipes[hash_function(recipe_name)];
            while(current->next != NULL){
                current = current->next;
            }
            current->next = new_recipe;
        }
        puts("aggiunta");
        return is_existing;
    }
    else{
        puts("ignorato");
        return is_existing;
    }
}

struct ingredient *search_ingredient(struct ingredient *ingredients, char ingredient_name[]){
    struct ingredient *current_ingredient = ingredients;
    while(current_ingredient != NULL && strcmp(current_ingredient->name, ingredient_name) != 0){
        current_ingredient = current_ingredient->next;
    }
    return current_ingredient;
}

struct shelf *search_shelf(struct shelf *warehouse[], char ingredient_name[]){
    struct shelf *current_shelf = warehouse[hash_function(ingredient_name)];
    while(current_shelf != NULL && strcmp(current_shelf->ingredient_name, ingredient_name) != 0){
        current_shelf = current_shelf->next;
    }
    return current_shelf;
}


void add_recipe_ingredient(struct recipe *recipes[], char name[], char ingredient[], int ingredient_quantity, struct shelf *warehouse[]){
    //check if the ingredient already exists
    struct recipe *selected_recipe = search_recipe(recipes, name);
    struct ingredient *new_ingredient = malloc(sizeof(struct ingredient));
    strcpy(new_ingredient->name, ingredient);
    new_ingredient->quantity = ingredient_quantity;
    new_ingredient->next = NULL;
    if(selected_recipe->ingredients == NULL){
        selected_recipe->ingredients = new_ingredient;
    }
    else{
        struct ingredient *current = selected_recipe->ingredients;
        while(current->next != NULL){
            current = current->next;
        }
        current->next = new_ingredient;
    }
    selected_recipe->weight = selected_recipe->weight + ingredient_quantity;
}

int is_in_orders(struct order orders[], char recipe_name[]){
    int is_in_orders = 0;
    int i = 0;
    while(orders[i].order_time != 0 && strcmp(orders[i].recipe_name, recipe_name) != 0){
        i++;
    }
    if(orders[i].order_time != 0){
        is_in_orders = 1;
        return is_in_orders;
    }
    else{
        return is_in_orders;
    }
}

void remove_recipe(struct recipe *recipes[], struct order orders[], struct order prepared[], char name[]){
    struct recipe *recipe = search_recipe(recipes, name);
    if(recipe != NULL){
        if(is_in_orders(orders, name) == 1 || is_in_orders(prepared, name) == 1){
            puts("ordini in sospeso");
        }
        else{
            if(recipe->ingredients != NULL){
                struct ingredient *current_ingredient = recipe->ingredients;
                struct ingredient *next_ingredient = recipe->ingredients;
                while(current_ingredient != NULL){
                    next_ingredient = current_ingredient->next;
                    free(current_ingredient);
                    current_ingredient = next_ingredient;
                }
            }
            if(recipes[hash_function(name)] == recipe){
                recipes[hash_function(name)] = recipe->next;
            }
            else{
                struct recipe *current_recipe = recipes[hash_function(name)];
                struct recipe *previous_recipe = recipes[hash_function(name)];
                while(current_recipe != recipe){
                    previous_recipe = current_recipe;
                    current_recipe = current_recipe->next;
                }
                previous_recipe->next = current_recipe->next;
            }
            free(recipe);
            puts("rimossa");
        }
    }
    else{
        puts("non presente");
    }
}

struct shelf *dequeue_stock(struct shelf *warehouse[], char ingredient[]){
    int index = hash_function(ingredient);
    struct shelf *current_shelf = warehouse[index];
    struct shelf *previous_shelf = NULL;
    while(strcmp(current_shelf->ingredient_name, ingredient) != 0){
        previous_shelf = current_shelf;
        current_shelf = current_shelf->next;
    }
    if(current_shelf->head == current_shelf->tail){
        if(previous_shelf == NULL){
            warehouse[index] = current_shelf->next;
        }
        else{
            previous_shelf->next = current_shelf->next;
        }
        free(current_shelf->head);
        free(current_shelf);
        return NULL;
    }
    else{
        struct stock *current_stock = current_shelf->head;
        current_shelf->head = current_shelf->head->next;
        current_shelf->total = current_shelf->total - current_stock->quantity;
        free(current_stock);
        return current_shelf;
    }
}

//order is already present in orders
void dequeue_order(struct order orders[], int order){
    int i = order;
    while(orders[i].order_time != 0){
        orders[i] = orders[i + 1];
        i++;
    }
}

void enqueue_stock(struct shelf *warehouse[], char ingredient[], struct stock *stock){
    int already_inserted = 0;
    struct shelf *current_shelf = search_shelf(warehouse, ingredient);
    if(current_shelf == NULL){
        struct shelf *new_shelf = malloc(sizeof(struct shelf));
        strcpy(new_shelf->ingredient_name, ingredient);
        new_shelf->total = stock->quantity;
        new_shelf->next = NULL;
        new_shelf->head = stock;
        new_shelf->tail = stock;
        current_shelf = warehouse[hash_function(ingredient)];
        if(current_shelf == NULL){
            warehouse[hash_function(ingredient)] = new_shelf;
        }
        else{
            struct shelf *tmp = current_shelf;
            while(tmp->next != NULL){
                tmp = tmp->next;
            }
            tmp->next = new_shelf;
        }
    }
    else{
        current_shelf->total = current_shelf->total + stock->quantity;
        //the stock of the ingredient has the maximum expiration
        if(stock->expiration > current_shelf->tail->expiration){
            current_shelf->tail->next = stock;
            current_shelf->tail = stock;
        }
        //the stock of the ingredient has the minimum expiration
        else if(stock->expiration < current_shelf->head->expiration){
            stock->next = current_shelf->head;
            current_shelf->head = stock;
        }
        else{
            struct stock *current_stock = current_shelf->head;
            struct stock *previous_stock = current_shelf->head;
            while(already_inserted == 0 && current_stock != NULL && stock->expiration >= current_stock->expiration){
                if(stock->expiration == current_stock->expiration){
                    current_stock->quantity = current_stock->quantity + stock->quantity;
                    already_inserted = 1;
                    free(stock);
                }
                previous_stock = current_stock;
                current_stock = current_stock->next;
            }
            if(already_inserted == 0){
                if(current_stock == NULL){
                    current_shelf->tail->next = stock;
                    current_shelf->tail = stock;
                }
                else{
                    if(previous_stock == NULL){
                        stock->next = current_shelf->head;
                        current_shelf->head = stock;
                    }
                    else{
                        previous_stock->next = stock;
                        stock->next = current_stock;
                    }
                }
            } 
        }
    }
}

struct shelf *check_expiration(struct shelf *warehouse[], struct shelf *shelf, int time){
    struct shelf *current_shelf = shelf;
    struct stock *current_stock;
    if(current_shelf == NULL){
        current_stock = NULL;
    }
    else{
        current_stock = shelf->head;
    }
    while(current_stock != NULL){
        if(current_stock->expiration <= time){
            current_shelf = dequeue_stock(warehouse, current_shelf->ingredient_name);
            if(current_shelf == NULL){
                break;
            }
            else{
                current_stock = current_shelf->head;
            }
        }
        else{
            break; 
        }
    }
    return current_shelf;
}
 
int check_stock(struct shelf *warehouse[], char ingredient[], int quantity, int time, struct shelf *shelf){
    if(shelf == NULL){
        return 0;
    }
    struct shelf *current_shelf = check_expiration(warehouse, shelf, time);
    if(current_shelf == NULL){
        return 0;
    }
    else{
        if(current_shelf->total < quantity){
            return 0;
        }
        else{
            return 1;
        }
    }
}

void use_stock(struct shelf *warehouse[], char ingredient[], int quantity, struct shelf *shelf){
    struct shelf *current_shelf = shelf;
    struct stock *current_stock = current_shelf->head;
    int decresable_quantity = quantity;
    while(decresable_quantity > 0){
        if(current_stock->quantity > decresable_quantity){
            current_stock->quantity = current_stock->quantity - decresable_quantity;
            current_shelf->total = current_shelf->total - decresable_quantity;
            decresable_quantity = 0;
        }
        else{
            decresable_quantity = decresable_quantity - current_stock->quantity;
            struct stock *next_stock = current_stock->next;
            dequeue_stock(warehouse, ingredient);
            current_stock = next_stock;
        }
    }
}

int preparation(struct shelf *warehouse[], struct order orders[], int order, struct recipe *recipes[], char recipe_name[], int time, int quantity){
    int shelves_indexes[50]; 
    int is_preparable = 0;
    struct ingredient *current_ingredient = orders[order].recipe->ingredients;
    int i = 0;
    while(current_ingredient != NULL){
        shelves_indexes[i] = hash_function(current_ingredient->name);
        is_preparable = check_stock(warehouse, current_ingredient->name, quantity*current_ingredient->quantity, time, warehouse[shelves_indexes[i]]);
        if(is_preparable == 0){
            return is_preparable;
        }
        current_ingredient = current_ingredient->next;
        i++;
    }
    current_ingredient = orders[order].recipe->ingredients;
    i = 0;
    while(current_ingredient != NULL){
        use_stock(warehouse, current_ingredient->name, quantity*current_ingredient->quantity, warehouse[shelves_indexes[i]]);
        current_ingredient = current_ingredient->next;
        i++;
    }
    dequeue_order(orders, order);
    return is_preparable;
}

void simple_insert_in_prepared(struct order prepared[], struct order package){
    int i = 0;
    while(prepared[i].order_time != 0){
        i++;
    }
    prepared[i] = package;
}

void complex_insert_in_prepared(struct order prepared[], struct order package, struct recipe *recipes[]){
    if(prepared[0].order_time == 0){
        strcpy(prepared[0].recipe_name, package.recipe_name);
        prepared[0].quantity = package.quantity;
        prepared[0].order_time = package.order_time;
        prepared[0].recipe = package.recipe;
        prepared[0].next = package.next;
    }
    else{
        int end = 0;
        while(prepared[end].order_time != 0){
            end++;
        }
        int i = 0;
        while(prepared[i].order_time != 0 && prepared[i].order_time < package.order_time){
            i++;
        }
        if(prepared[i].order_time == 0){
            strcpy(prepared[i].recipe_name, package.recipe_name);
            prepared[i].quantity = package.quantity;
            prepared[i].order_time = package.order_time;
            prepared[i].recipe = package.recipe;
            prepared[i].next = package.next;
        }
        else{
            int j = end;
            while(j >= i){
                prepared[j + 1] = prepared[j];
                j--;
            }
            strcpy(prepared[i].recipe_name, package.recipe_name);
            prepared[i].quantity = package.quantity;
            prepared[i].order_time = package.order_time;
            prepared[i].recipe = package.recipe;
            prepared[i].next = package.next;
        }
    }
}

int check_orders_after_supply(struct order orders[], struct shelf *warehouse[], struct recipe *recipes[], struct order prepared[], int time, int num_orders){
    int num = num_orders;
    int is_not_empty = 0;
    for(int i = 0; i < MAX_SHELVES; i++){
        if(warehouse[i] != NULL){
            is_not_empty = 1;
            break;
        }
    }
    if(is_not_empty == 1 && orders[0].order_time != 0){
        int i = 0;
        while(orders[i].order_time != 0){
            struct order new_package;
            strcpy(new_package.recipe_name, orders[i].recipe_name);
            new_package.quantity = orders[i].quantity;
            new_package.order_time = orders[i].order_time;
            new_package.recipe = orders[i].recipe;
            new_package.next = NULL;
            int is_preparable = preparation(warehouse, orders, i, recipes, orders[i].recipe_name, time, orders[i].quantity);
            if(is_preparable != 0){
                complex_insert_in_prepared(prepared, new_package, recipes);
                num = num - 1;
            }
            else{
                i++;
            }
        }
    } 
    return num;
}

int save_order(struct shelf *warehouse[], struct order orders[], struct order prepared[], char recipe_name[], int quantity, int time, struct recipe *recipes[], int courier_max, int num_orders){
    int num = num_orders;
    struct recipe *recipe = search_recipe(recipes, recipe_name);
    if(recipe == NULL){
        puts("rifiutato");
    }
    else{
        struct order new_order;
        strcpy(new_order.recipe_name, recipe_name);
        new_order.quantity = quantity;
        new_order.order_time = time;
        new_order.recipe = recipe;
        new_order.next = NULL;
        orders[num_orders] = new_order;
        num = num + 1;
        int is_preparable = preparation(warehouse, orders, num_orders, recipes, recipe_name, time, quantity);
        if(is_preparable != 0){
            struct order new_package;
            strcpy(new_package.recipe_name, recipe_name);
            new_package.quantity = quantity;
            new_package.order_time = time;
            new_package.recipe = recipe;
            new_package.next = NULL;
            simple_insert_in_prepared(prepared, new_package);
            num = num - 1;
        }
        puts("accettato");
    }
    return num;
}

void supply(struct shelf *warehouse[], char ingredient[], int quantity, int expiration, int time){
    struct stock *new_stock = malloc(sizeof(struct stock));
    new_stock->expiration = expiration;
    new_stock->quantity = quantity;
    new_stock->next = NULL;
    if(warehouse[hash_function(ingredient)] == NULL){
        struct shelf *new_shelf = malloc(sizeof(struct shelf));
        strcpy(new_shelf->ingredient_name, ingredient);
        new_shelf->total = quantity;
        new_shelf->head = new_stock;
        new_shelf->tail = new_stock;
        new_shelf->next = NULL;
        warehouse[hash_function(ingredient)] = new_shelf;
    }
    else{
        enqueue_stock(warehouse, ingredient, new_stock);
    }
}

void dequeue_prepared(struct order prepared[], int package){
    int i = package;
    while(prepared[i].order_time != 0){
        prepared[i] = prepared[i + 1];
        i++;
    }
}

void max_heapify(struct order *sorted_spedition, int spedition_size, int heap_size, int index, struct recipe *recipes[]){
    int left = 2*index + 1;
    int right = 2*index + 2;
    int max = index;
    int current_weight = sorted_spedition[index].quantity*search_recipe(recipes, sorted_spedition[index].recipe_name)->weight;

    if(left < heap_size) {
        int left_weight = sorted_spedition[left].quantity*search_recipe(recipes, sorted_spedition[left].recipe_name)->weight;
        if(left_weight > current_weight || (left_weight == current_weight && sorted_spedition[left].order_time < sorted_spedition[index].order_time)) {
            max = left;
            current_weight = left_weight;
        }
    }

    if(right < heap_size) {
        int right_weight = sorted_spedition[right].quantity*search_recipe(recipes, sorted_spedition[right].recipe_name)->weight;
        if(right_weight > current_weight || (right_weight == current_weight && sorted_spedition[right].order_time < sorted_spedition[max].order_time)) {
            max = right;
        }
    }

    if(max != index) {
        struct order tmp = sorted_spedition[index];
        sorted_spedition[index] = sorted_spedition[max];
        sorted_spedition[max] = tmp;
        max_heapify(sorted_spedition, spedition_size, heap_size, max, recipes);
    }
}

int build_max_heap(struct order *sorted_spedition, int spedition_size, struct recipe *recipes[]){
    int heap_size = spedition_size;
    for(int i = (spedition_size/2) - 1; i >= 0; i--){
        max_heapify(sorted_spedition, spedition_size, heap_size, i, recipes);
    }
    return heap_size;
}

void heapsort(struct order *sorted_spedition, int spedition_size, struct recipe *recipes[]){
    int heap_size = build_max_heap(sorted_spedition, spedition_size, recipes);
    for(int i = spedition_size - 1; i > 0; i--){
        struct order tmp = sorted_spedition[0];
        sorted_spedition[0] = sorted_spedition[i];
        sorted_spedition[i] = tmp;
        heap_size = heap_size - 1;
        max_heapify(sorted_spedition, spedition_size, heap_size, 0, recipes);
    }
}

int is_in_spedition(struct orders *spedition, char recipe_name[]){
    int is_in_spedition = 0;
    struct order *current_order = spedition->head;
    while(current_order != NULL && strcmp(current_order->recipe_name, recipe_name) != 0){
        current_order = current_order->next;
    }
    if(current_order != NULL){
        is_in_spedition = 1;
        return is_in_spedition;
    }
    else{
        return is_in_spedition;
    }
}

void insert_in_spedition(struct orders *spedition, int package, struct order prepared[]){
    struct order *new_spedition = malloc(sizeof(struct order));
    strcpy(new_spedition->recipe_name, prepared[package].recipe_name);
    new_spedition->quantity = prepared[package].quantity;
    new_spedition->order_time = prepared[package].order_time;
    new_spedition->next = NULL;
    if(spedition->head == NULL){
        spedition->head = spedition->tail = new_spedition;
    }
    else{
        spedition->tail->next = new_spedition;
        spedition->tail = new_spedition;
    }
    dequeue_prepared(prepared, package);
}

void spedition(struct order prepared[], int courier_max, struct recipe *recipes[]){
    if(prepared[0].order_time != 0){
        struct orders *spedition = malloc(sizeof(struct orders));
        spedition->head = NULL;
        spedition->tail = NULL;
        int spedition_size = 0;
        int i = 0;
        int spedition_weight = 0;
        while(prepared[i].order_time != 0){
            int current_weight = prepared[i].quantity*prepared[i].recipe->weight;
            if(spedition_weight + current_weight > courier_max){
                break;
            }
            else{
                spedition_weight = spedition_weight + current_weight;
                spedition_size++;
                insert_in_spedition(spedition, i, prepared);
            }
        }
        struct order *sorted_spedition = malloc(spedition_size * sizeof(struct order));
        struct order *current_spedition = spedition->head;
        for(int i = 0; i < spedition_size; i++) {
            sorted_spedition[i] = *current_spedition;
            current_spedition = current_spedition->next;
        }
        current_spedition = spedition->head;
        while(current_spedition != NULL){
            struct order *next_spedition = current_spedition->next;
            free(current_spedition);
            current_spedition = next_spedition;
        }
        free(spedition);
        free(current_spedition);
        heapsort(sorted_spedition, spedition_size, recipes);
        for(int i = spedition_size - 1; i >= 0; i--){
            printf("%d %s %d\n", sorted_spedition[i].order_time, sorted_spedition[i].recipe_name, sorted_spedition[i].quantity);
        }  
        free(sorted_spedition);
    }      
    else{
        puts("camioncino vuoto");
    }    
}

int main(){

    int courier_frequency;
    int courier_max;

    int check = 0;

    char command[21];
    char recipe_name[21];
    char ingredient[21];
    int ingredient_quantity;
    int ingredient_expiration;
    int recipe_quantity;

    struct recipe *recipes[MAX_RECIPES] = {NULL};
    struct shelf *warehouse[MAX_SHELVES] = {NULL};
    struct order orders[MAX_ORDERS];
    int num_orders = 0;
    for(int i = 0; i < MAX_ORDERS; i++){
        orders[i].order_time = 0;
    }
    struct order prepared[MAX_ORDERS];
    for(int i = 0; i < MAX_ORDERS; i++){
        prepared[i].order_time = 0;
    }

    check = scanf("%d", &courier_frequency);
    check = scanf("%d", &courier_max);
    
    int time = 0;

    check = scanf("%s", command);
    while(check != EOF){ 
        if(time != 0 && time % courier_frequency == 0){
            spedition(prepared, courier_max, recipes);
        }

        if(command[2] == 'g'){
            check = scanf("%s", recipe_name);
            int is_existing = insert_recipe(recipes, recipe_name);
            check = scanf("%s", ingredient);
            while(check != EOF && (strcmp(ingredient, "aggiungi_ricetta") != 0) && (strcmp(ingredient, "rimuovi_ricetta") != 0) && (strcmp(ingredient, "rifornimento") != 0) && (strcmp(ingredient, "ordine") != 0)){
                check = scanf("%d", &ingredient_quantity);
                if(is_existing == 0){
                    add_recipe_ingredient(recipes, recipe_name, ingredient, ingredient_quantity, warehouse);
                }
                check = scanf("%s", ingredient);
            }
            strcpy(command, ingredient);
            time++;
        }
        else if(command[2] == 'm'){
            check = scanf("%s", recipe_name);
            remove_recipe(recipes, orders, prepared, recipe_name);
            check = scanf("%s", command);
            time++;
        }
        else if(command[2] == 'f'){
            check = scanf("%s", ingredient);
            while(check != EOF && (strcmp(ingredient, "aggiungi_ricetta") != 0) && (strcmp(ingredient, "rimuovi_ricetta") != 0) && (strcmp(ingredient, "rifornimento") != 0) && (strcmp(ingredient, "ordine") != 0)){
                check = scanf("%d", &ingredient_quantity);
                check = scanf("%d", &ingredient_expiration);
                if(ingredient_expiration > time){
                    supply(warehouse, ingredient, ingredient_quantity, ingredient_expiration, time);
                }
                check = scanf("%s", ingredient);
            }
            num_orders = check_orders_after_supply(orders, warehouse, recipes, prepared, time, num_orders);
            time++; 
            puts("rifornito");
            strcpy(command, ingredient);
        }
        else if(command[2] == 'd'){
            check = scanf("%s", recipe_name);
            check = scanf("%d", &recipe_quantity);
            num_orders = save_order(warehouse, orders, prepared, recipe_name, recipe_quantity, time, recipes, courier_max, num_orders);
            check = scanf("%s", command);
            time++;
        }
    }
    if(time % courier_frequency == 0){
        spedition(prepared, courier_max, recipes);
    }

    //free the remaining allocated memory
    for(int i = 0; i < MAX_RECIPES; i++){
        struct recipe *current_recipe = recipes[i];
        while(current_recipe != NULL){
            struct ingredient *current_ingredient = current_recipe->ingredients;
            while(current_ingredient != NULL){
                struct ingredient *next_ingredient = current_ingredient->next;
                free(current_ingredient);
                current_ingredient = next_ingredient;
            }
            struct recipe *next_recipe = current_recipe->next;
            free(current_recipe); 
            current_recipe = next_recipe;
        }
    }

    for(int i = 0; i < MAX_SHELVES; i++){
        struct shelf *current_shelf = warehouse[i];
        while(current_shelf != NULL){
            struct stock *current_stock = current_shelf->head;
            while(current_stock != NULL){
                struct stock *next_stock = current_stock->next;
                free(current_stock);
                current_stock = next_stock;
            }
            struct shelf *next_shelf = current_shelf->next;
            free(current_shelf);
            current_shelf = next_shelf;
        }
    }

    return 0;
}