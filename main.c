#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PRODUCTS 20
#define MAX_WAREHOUSE 10
#define MAX_BATCHES 50

/* -------------------- STRUCTURES -------------------- */

typedef struct {
    int id;
    char name[50];
    int aisle;
    int shelf;
    float weight;
    float volume;
} Product;

typedef struct {
    int productId;
    int quantity;
} OrderItem;

typedef struct {
    int id;
    OrderItem *items;
    int itemCount;
} Order;

typedef struct {
    int id;
    int *orderIds;
    int orderCount;
    float totalWeight;
    float totalVolume;
} Batch;

typedef struct {
    int productId;
    int quantity;
    int x;
    int y;
} PickPoint;

/* -------------------- GLOBAL DATA -------------------- */

Product products[MAX_PRODUCTS];
Order *orders = NULL;
Batch batches[MAX_BATCHES];

int productCount = 0;
int orderCount = 0;
int batchCount = 0;

int warehouse[MAX_WAREHOUSE][MAX_WAREHOUSE];

/* -------------------- FUNCTION DECLARATIONS -------------------- */

void initializeProducts();
void initializeWarehouse();

Product *findProduct(int id);
Order *findOrder(int id);

void displayProducts();
void displayWarehouse();
void addOrder();
void displayOrders();

float orderWeight(Order *order);
float orderVolume(Order *order);

void createBatches(float maxWeight, float maxVolume);
void displayBatches();

float distance(int x1, int y1, int x2, int y2);

int createPickPoints(Batch *batch, PickPoint **points);

float routePickPoints(PickPoint *points, int count, int showRoute);

float singleOrderCost(Order *order);
float batchedPickingCost();

void comparePicking();

void freeMemory();

/* -------------------- MAIN -------------------- */

int main() {
    int choice;
    float maxWeight, maxVolume;

    initializeProducts();
    initializeWarehouse();

    while (1) {
        printf("\n============================================\n");
        printf("   WAREHOUSE ORDER PICKING OPTIMIZATION\n");
        printf("============================================\n");
        printf("1. Display Products\n");
        printf("2. Display Warehouse Layout\n");
        printf("3. Add Order\n");
        printf("4. Display Orders\n");
        printf("5. Create Batches\n");
        printf("6. Display Batch Manifests\n");
        printf("7. Show Optimized Pick Routes\n");
        printf("8. Compare Single vs Batched Picking\n");
        printf("9. Exit\n");
        printf("--------------------------------------------\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {

        case 1:
            displayProducts();
            break;

        case 2:
            displayWarehouse();
            break;

        case 3:
            addOrder();
            break;

        case 4:
            displayOrders();
            break;

        case 5:
            if (orderCount == 0) {
                printf("\nNo orders available.\n");
                break;
            }

            printf("\nEnter maximum batch weight: ");
            scanf("%f", &maxWeight);

            printf("Enter maximum batch volume: ");
            scanf("%f", &maxVolume);

            createBatches(maxWeight, maxVolume);
            displayBatches();
            break;

        case 6:
            displayBatches();
            break;

        case 7:
            if (batchCount == 0) {
                printf("\nCreate batches first.\n");
                break;
            }

            printf("\n========== OPTIMIZED PICK ROUTES ==========\n");

            for (int i = 0; i < batchCount; i++) {

                PickPoint *points = NULL;
                int count = createPickPoints(&batches[i], &points);

                printf("\nBatch %d\n", batches[i].id);

                if (count == 0) {
                    printf("No products to pick.\n");
                } else {
                    float cost = routePickPoints(points, count, 1);
                    printf("Estimated travel distance: %.2f units\n", cost);
                }

                free(points);
            }
            break;

        case 8:
            if (orderCount == 0) {
                printf("\nNo orders available.\n");
                break;
            }

            comparePicking();
            break;

        case 9:
            freeMemory();
            printf("\nProgram terminated successfully.\n");
            return 0;

        default:
            printf("\nInvalid choice. Try again.\n");
        }
    }

    return 0;
}

/* -------------------- INITIALIZE PRODUCTS -------------------- */

void initializeProducts() {

    Product sample[] = {
        {1, "Laptop",       1, 2, 2.50, 3.00},
        {2, "Keyboard",     2, 4, 0.80, 1.00},
        {3, "Mouse",        3, 2, 0.30, 0.50},
        {4, "Headphones",   4, 5, 0.50, 1.20},
        {5, "Monitor",      5, 3, 4.00, 5.00},
        {6, "USB Cable",    6, 2, 0.20, 0.30},
        {7, "Webcam",       7, 4, 0.40, 0.80},
        {8, "Printer",      8, 6, 6.00, 8.00}
    };

    productCount = 8;

    for (int i = 0; i < productCount; i++) {
        products[i] = sample[i];
    }
}

/* -------------------- INITIALIZE WAREHOUSE -------------------- */

void initializeWarehouse() {

    for (int i = 0; i < MAX_WAREHOUSE; i++) {
        for (int j = 0; j < MAX_WAREHOUSE; j++) {
            warehouse[i][j] = 0;
        }
    }

    for (int i = 0; i < productCount; i++) {
        int x = products[i].aisle;
        int y = products[i].shelf;

        if (x < MAX_WAREHOUSE && y < MAX_WAREHOUSE) {
            warehouse[x][y] = products[i].id;
        }
    }
}

/* -------------------- FIND PRODUCT -------------------- */

Product *findProduct(int id) {

    for (int i = 0; i < productCount; i++) {
        if (products[i].id == id)
            return &products[i];
    }

    return NULL;
}

/* -------------------- FIND ORDER -------------------- */

Order *findOrder(int id) {

    for (int i = 0; i < orderCount; i++) {
        if (orders[i].id == id)
            return &orders[i];
    }

    return NULL;
}

/* -------------------- DISPLAY PRODUCTS -------------------- */

void displayProducts() {

    printf("\n================ PRODUCTS ================\n");

    printf("%-5s %-15s %-8s %-8s %-10s %-10s\n",
           "ID", "Product", "Aisle", "Shelf",
           "Weight", "Volume");

    printf("--------------------------------------------\n");

    for (int i = 0; i < productCount; i++) {

        printf("%-5d %-15s %-8d %-8d %-10.2f %-10.2f\n",
               products[i].id,
               products[i].name,
               products[i].aisle,
               products[i].shelf,
               products[i].weight,
               products[i].volume);
    }
}

/* -------------------- DISPLAY WAREHOUSE -------------------- */

void displayWarehouse() {

    printf("\n=========== WAREHOUSE LAYOUT ===========\n");
    printf("0 = Empty location\n\n");

    for (int i = 0; i < MAX_WAREHOUSE; i++) {

        for (int j = 0; j < MAX_WAREHOUSE; j++) {
            printf("%3d ", warehouse[i][j]);
        }

        printf("\n");
    }
}

/* -------------------- ADD ORDER -------------------- */

void addOrder() {

    int n;

    orders = realloc(orders, (orderCount + 1) * sizeof(Order));

    if (orders == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    Order *newOrder = &orders[orderCount];

    newOrder->id = orderCount + 1;
    newOrder->items = NULL;
    newOrder->itemCount = 0;

    printf("\n========== ADD ORDER %d ==========\n", newOrder->id);

    printf("Enter number of different products: ");
    scanf("%d", &n);

    if (n <= 0) {
        printf("Invalid number of products.\n");
        return;
    }

    newOrder->items = malloc(n * sizeof(OrderItem));

    if (newOrder->items == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    for (int i = 0; i < n; i++) {

        int productId, quantity;

        printf("\nEnter product ID: ");
        scanf("%d", &productId);

        Product *p = findProduct(productId);

        if (p == NULL) {
            printf("Invalid product ID. Try again.\n");
            i--;
            continue;
        }

        printf("Enter quantity: ");
        scanf("%d", &quantity);

        if (quantity <= 0) {
            printf("Invalid quantity.\n");
            i--;
            continue;
        }

        newOrder->items[i].productId = productId;
        newOrder->items[i].quantity = quantity;
        newOrder->itemCount++;
    }

    orderCount++;

    printf("\nOrder %d added successfully.\n", newOrder->id);
}

/* -------------------- DISPLAY ORDERS -------------------- */

void displayOrders() {

    if (orderCount == 0) {
        printf("\nNo orders available.\n");
        return;
    }

    printf("\n================ ORDERS ================\n");

    for (int i = 0; i < orderCount; i++) {

        printf("\nOrder ID: %d\n", orders[i].id);

        printf("%-10s %-15s %-10s\n",
               "Product", "Name", "Quantity");

        for (int j = 0; j < orders[i].itemCount; j++) {

            Product *p = findProduct(
                orders[i].items[j].productId
            );

            if (p != NULL) {

                printf("%-10d %-15s %-10d\n",
                       p->id,
                       p->name,
                       orders[i].items[j].quantity);
            }
        }

        printf("Total Weight: %.2f\n",
               orderWeight(&orders[i]));

        printf("Total Volume: %.2f\n",
               orderVolume(&orders[i]));
    }
}

/* -------------------- ORDER WEIGHT -------------------- */

float orderWeight(Order *order) {

    float total = 0;

    for (int i = 0; i < order->itemCount; i++) {

        Product *p = findProduct(
            order->items[i].productId
        );

        if (p != NULL) {
            total += p->weight *
                     order->items[i].quantity;
        }
    }

    return total;
}

/* -------------------- ORDER VOLUME -------------------- */

float orderVolume(Order *order) {

    float total = 0;

    for (int i = 0; i < order->itemCount; i++) {

        Product *p = findProduct(
            order->items[i].productId
        );

        if (p != NULL) {
            total += p->volume *
                     order->items[i].quantity;
        }
    }

    return total;
}

/* -------------------- CREATE BATCHES -------------------- */

void createBatches(float maxWeight, float maxVolume) {

    batchCount = 0;

    for (int i = 0; i < orderCount; i++) {

        float w = orderWeight(&orders[i]);
        float v = orderVolume(&orders[i]);

        if (w > maxWeight || v > maxVolume) {

            printf("\nOrder %d cannot fit into a batch.",
                   orders[i].id);

            printf(" Weight = %.2f, Volume = %.2f\n",
                   w, v);

            continue;
        }

        int placed = 0;

        for (int j = 0; j < batchCount; j++) {

            if (batches[j].totalWeight + w <= maxWeight &&
                batches[j].totalVolume + v <= maxVolume) {

                batches[j].orderIds =
                    realloc(
                        batches[j].orderIds,
                        (batches[j].orderCount + 1)
                        * sizeof(int)
                    );

                batches[j].orderIds[
                    batches[j].orderCount
                ] = orders[i].id;

                batches[j].orderCount++;

                batches[j].totalWeight += w;
                batches[j].totalVolume += v;

                placed = 1;
                break;
            }
        }

        if (!placed && batchCount < MAX_BATCHES) {

            batches[batchCount].id = batchCount + 1;

            batches[batchCount].orderIds =
                malloc(sizeof(int));

            batches[batchCount].orderIds[0] =
                orders[i].id;

            batches[batchCount].orderCount = 1;
            batches[batchCount].totalWeight = w;
            batches[batchCount].totalVolume = v;

            batchCount++;
        }
    }

    printf("\nBatching completed successfully.\n");
}

/* -------------------- DISPLAY BATCHES -------------------- */

void displayBatches() {

    if (batchCount == 0) {
        printf("\nNo batches created.\n");
        return;
    }

    printf("\n=============== BATCH MANIFESTS ===============\n");

    for (int i = 0; i < batchCount; i++) {

        printf("\nBatch %d\n", batches[i].id);

        printf("Orders: ");

        for (int j = 0; j < batches[i].orderCount; j++) {
            printf("%d ", batches[i].orderIds[j]);
        }

        printf("\n");

        printf("Total Weight : %.2f\n",
               batches[i].totalWeight);

        printf("Total Volume : %.2f\n",
               batches[i].totalVolume);
    }
}

/* -------------------- MANHATTAN DISTANCE -------------------- */

float distance(int x1, int y1, int x2, int y2) {

    int dx = abs(x1 - x2);
    int dy = abs(y1 - y2);

    return dx + dy;
}

/* -------------------- CREATE PICK POINTS -------------------- */

int createPickPoints(Batch *batch, PickPoint **points) {

    int capacity = 5;
    int count = 0;

    *points = malloc(capacity * sizeof(PickPoint));

    if (*points == NULL) {
        return 0;
    }

    for (int i = 0; i < batch->orderCount; i++) {

        Order *order =
            findOrder(batch->orderIds[i]);

        if (order == NULL)
            continue;

        for (int j = 0; j < order->itemCount; j++) {

            int productId =
                order->items[j].productId;

            int quantity =
                order->items[j].quantity;

            int found = 0;

            /* Combine same product from different orders */
            for (int k = 0; k < count; k++) {

                if ((*points)[k].productId == productId) {

                    (*points)[k].quantity += quantity;
                    found = 1;
                    break;
                }
            }

            if (found)
                continue;

            if (count >= capacity) {

                capacity *= 2;

                *points =
                    realloc(
                        *points,
                        capacity * sizeof(PickPoint)
                    );

                if (*points == NULL)
                    return 0;
            }

            Product *p = findProduct(productId);

            if (p != NULL) {

                (*points)[count].productId =
                    productId;

                (*points)[count].quantity =
                    quantity;

                (*points)[count].x =
                    p->aisle;

                (*points)[count].y =
                    p->shelf;

                count++;
            }
        }
    }

    return count;
}

/* -------------------- NEAREST NEIGHBOR ROUTING -------------------- */

float routePickPoints(
    PickPoint *points,
    int count,
    int showRoute
) {

    if (count == 0)
        return 0;

    int *visited =
        calloc(count, sizeof(int));

    if (visited == NULL)
        return 0;

    int currentX = 0;
    int currentY = 0;

    float totalDistance = 0;

    if (showRoute) {
        printf("Start: (0,0)\n");
    }

    for (int step = 0; step < count; step++) {

        int nearest = -1;
        float nearestDistance = 999999;

        for (int i = 0; i < count; i++) {

            if (!visited[i]) {

                float d =
                    distance(
                        currentX,
                        currentY,
                        points[i].x,
                        points[i].y
                    );

                if (d < nearestDistance) {
                    nearestDistance = d;
                    nearest = i;
                }
            }
        }

        if (nearest == -1)
            break;

        visited[nearest] = 1;

        totalDistance += nearestDistance;

        if (showRoute) {

            Product *p =
                findProduct(points[nearest].productId);

            printf(" -> %s (Qty: %d) [%d,%d] ",
                   p->name,
                   points[nearest].quantity,
                   points[nearest].x,
                   points[nearest].y);

            printf("+%.0f units\n",
                   nearestDistance);
        }

        currentX = points[nearest].x;
        currentY = points[nearest].y;
    }

    /* Return to packing/start point */
    totalDistance +=
        distance(currentX, currentY, 0, 0);

    if (showRoute) {
        printf(" -> Return to (0,0)\n");
    }

    free(visited);

    return totalDistance;
}

/* -------------------- SINGLE ORDER COST -------------------- */

float singleOrderCost(Order *order) {

    PickPoint *points = NULL;

    Batch tempBatch;

    tempBatch.id = 0;
    tempBatch.orderIds = &order->id;
    tempBatch.orderCount = 1;
    tempBatch.totalWeight = orderWeight(order);
    tempBatch.totalVolume = orderVolume(order);

    int count =
        createPickPoints(&tempBatch, &points);

    float cost =
        routePickPoints(points, count, 0);

    free(points);

    return cost;
}

/* -------------------- BATCHED PICKING COST -------------------- */

float batchedPickingCost() {

    float total = 0;

    for (int i = 0; i < batchCount; i++) {

        PickPoint *points = NULL;

        int count =
            createPickPoints(
                &batches[i],
                &points
            );

        total +=
            routePickPoints(points, count, 0);

        free(points);
    }

    return total;
}

/* -------------------- SCENARIO COMPARISON -------------------- */

void comparePicking() {

    float singleCost = 0;

    printf("\n========== SCENARIO COMPARISON ==========\n");

    for (int i = 0; i < orderCount; i++) {

        float cost =
            singleOrderCost(&orders[i]);

        printf("Order %d travel cost: %.2f\n",
               orders[i].id,
               cost);

        singleCost += cost;
    }

    printf("\nTotal single-order picking cost: %.2f\n",
           singleCost);

    if (batchCount == 0) {

        printf("\nNo batches available.\n");
        printf("Create batches to compare results.\n");

        return;
    }

    float batchCost =
        batchedPickingCost();

    printf("Total batched picking cost: %.2f\n",
           batchCost);

    if (singleCost > 0) {

        float saving =
            ((singleCost - batchCost)
             / singleCost) * 100;

        printf("\nTravel saving: %.2f%%\n",
               saving);

        if (saving > 0)
            printf("Batched picking is more efficient.\n");
        else
            printf("Single-order picking is more efficient for this data.\n");
    }
}

/* -------------------- FREE MEMORY -------------------- */

void freeMemory() {

    for (int i = 0; i < orderCount; i++) {
        free(orders[i].items);
    }

    free(orders);

    for (int i = 0; i < batchCount; i++) {
        free(batches[i].orderIds);
    }
}
