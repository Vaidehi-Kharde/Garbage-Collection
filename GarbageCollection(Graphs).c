#include <stdio.h>
#include <stdlib.h>

typedef struct metadata
{
    struct metadata *next;
    int size;
} metadata;

#define MAX 5
typedef int Vertex;

typedef struct Edge_tag
{
    Vertex endPoint;
    int marked;
    struct Edge_tag *nextEdge;
} Edge ;

typedef struct Graph_tag
{
    Edge *EdgeList[MAX];
    int marked[MAX];
    int references[MAX];
}Graph; 

typedef struct Node_tag
{
    int data;
    Edge *GraphNode;
    struct Node_tag* next;
} Node;

char memory[10000]; // taking a memory of 10000 bytes from the stack
metadata *freeList = (metadata *)memory;
// memory points to the 1st location of the array
// deallocate list also points to the 1st location of the array considering it as a metadata

void merge(metadata *prev)
{
    if (prev != NULL && prev->next != NULL)
    {
        metadata *ptr = prev->next;
        // merging with the next node
        if (ptr->next != NULL && (void *)ptr + ptr->size + sizeof(metadata) == ptr->next)
        {
            ptr->size = ptr->size + (ptr->next)->size + sizeof(metadata);
            ptr->next = ptr->next->next;
            ptr->next = NULL;
        }
        // merging with the previous node
        if ((void *)prev + prev->size + sizeof(metadata) == ptr)
        {
            prev->next = ptr->next;
            prev->size = prev->size + ptr->size + sizeof(metadata);
            ptr = NULL;
        }
    }
}

void deallocate(void *ptr)
{
    if (ptr == NULL)
    {
        printf("\nInvalid block");
    }
    else
    {
        metadata *temp = freeList, *prev;
        // find where the deallocate block should be inserted
        while (ptr > (void *)temp)
        {
            prev = temp;
            temp = temp->next;
        }
        if (freeList == temp) // insertion in freeList before first node
        {
            ((metadata *)ptr)->next = freeList;
            freeList = (metadata *)ptr;
        }
        else
        {
            ((metadata *)ptr)->next = temp;
            prev->next = (metadata *)ptr;
        }
        temp = freeList;
        printf("\n\nBefore merging: ");
        while (temp != NULL)
        {
            printf("\naddress: %p, size: %d", temp, temp->size);
            temp = temp->next;
        }
        merge(prev);
        printf("\n\nAfter Merging: ");
        temp = freeList;
        while (temp != NULL)
        {
            printf("\naddress: %p, size: %d", temp, temp->size);
            temp = temp->next;
        }
        printf ("\nDeallocated");
    }
}

void *allocate(int blockSize)
{
    void *res;
    metadata *ptr;
    if (freeList->next == NULL && (int)(blockSize + sizeof(metadata)) < freeList->size) // single deallocate block
    {
        ptr = freeList;
        //moving by some bytes
        freeList = (metadata *)((void *)freeList + blockSize + sizeof(metadata));
        // restoring values of freeList
        freeList->size = ptr->size - sizeof(metadata) - blockSize;
        freeList->next = NULL;
        // while comparing, compare blockSize + sizeof(metadata), but while storing in the meta data, ignore size of metadata for better user readability
        ptr->size = blockSize;
        ptr->next = NULL;
        printf("\n\nAllocated! ");
    }
    else
    {
        // allocating in freed chunks, or last remaining large chunk
        metadata *temp = freeList, *prev = freeList;
        // find an appropriate sized chunk
        while (temp != NULL && temp->size < (int)(blockSize + sizeof(metadata)))
        {
            prev = temp;
            temp = temp->next;
        }
        if (temp == NULL) // array is finished, cause can be external fragmentation
        {
            printf("No memory");
            res = NULL;
        }
        else // a chunk is found(can be first or last or in between)
        {
            ptr = (void *)temp; // points to the starting address of thet chunk
            if (prev == temp)   // changing deallocate pointer if memory is allocated at the start
            {
                freeList = (metadata *)((void *)temp + blockSize + sizeof(metadata));
            }
            temp = (metadata *)((void *)temp + blockSize + sizeof(metadata)); // shifting the metabadata of the deallocate chunk, making space for the memory to be allocated
            // restoring size and next pointer of temp(the deallocate chunk which was moved)
            temp->size = ((metadata *)ptr)->size - blockSize - sizeof(metadata); // absolute size of the meomory chunk, metadata included
            temp->next = ((metadata *)ptr)->next;
            // setting metadata for the newly allocated memry from the chunk
            ((metadata *)ptr)->size = blockSize;
            ((metadata *)ptr)->next = NULL;
            if (prev != temp) // if its not the starting part
            {
                prev->next = temp;
            }
            printf("\n\nAllocated");
        }
    }
    res = (void *)ptr;
    return res;
}
void initialaize(Graph *graph)
{
    for (int i = 0; i < MAX; i++)
    {
        graph->EdgeList[i] = NULL;
        graph->marked[i] = 0;
    }
}

void insertInGraph(Graph *graph, Vertex v1, Vertex v2)
{
    Edge *new1 = (Edge *)allocate (sizeof(Edge));
    new1->marked = 0;
    new1->endPoint = v1;
    Edge *new2 = (Edge *)allocate (sizeof(Edge));
    new2->endPoint = v2;
    new2->marked = 0;

    //Edge v1-----v2
    Edge *ptr = graph->EdgeList[v1];
    Edge *prev = graph->EdgeList[v1];
    new2->nextEdge = NULL;
    if (ptr == NULL)
    {
        graph->EdgeList[v1] = new2;
    }
    else
    {
        while (ptr != NULL && ptr->endPoint < v2 )
        {
            prev = ptr;
            ptr = ptr->nextEdge;
        }
        if (ptr != NULL && ptr->endPoint == v2) 
        {
            printf ("Duplicate edge");
            deallocate (new1);
            deallocate (new2);
        }
        else
        {
            new2->nextEdge = ptr;
            if (ptr == graph->EdgeList[v1])
            {
                graph->EdgeList[v1] = new2;
            }
            else
            {
                prev->nextEdge = new2;
            }
        }
    }
}

void printList (Graph *graph)
{
    for (int i = 0; i < MAX; i++)
    {
        printf ("%d : ", i);
        Edge *ptr = graph->EdgeList[i];
        while (ptr != NULL)
        {
            printf ("%d ", ptr->endPoint);
            ptr = ptr->nextEdge;
        }
        printf ("\n");
    }
}

void printAdjacencyMatrix(Graph *graph)
{
    int arr[MAX][MAX];
    for (int i = 0; i < MAX; i++)
    {
        for (int j = 0; j < MAX; j++)
        {
            arr[i][j] = 0;
        }
    }

    for (int i = 0; i < MAX; i++)
    {
        Edge *ptr = graph->EdgeList[i];
        while (ptr != NULL)
        {
            arr[i][ptr->endPoint] = 1;
            ptr = ptr->nextEdge;
        }
        printf ("\n");
    }

    for (int i = 0; i < MAX; i++)
    {
        for (int j = 0; j < MAX; j++)
        {
            printf ("%d ", arr[i][j]);
        }
        printf ("\n");
    }
}

Edge *searchForVertex(Graph *graph, Vertex key, Edge *current, Vertex visited[])
{
    if (current != NULL)
    {
        if (current->endPoint == key)
        {
            return current;
        }
        visited[current->endPoint] = 1;
        Edge *ptr = graph->EdgeList[current->endPoint];

        while (ptr != NULL) 
        {
            if (visited[ptr->endPoint] == 0) 
            {
                Edge * retVal = searchForVertex(graph, key, ptr, visited);
                if (retVal != NULL) return retVal;
            }
            ptr = ptr->nextEdge;
        }
    }
    return NULL;
}

void marking (Graph *graph, Edge *graphNode)
{
    graph->marked[graphNode->endPoint] = 1;
    Edge *ptr = graph->EdgeList[graphNode->endPoint];

    while (ptr != NULL) 
    {
        if (ptr->marked == 0) 
        {
            marking (graph, ptr);
        }
        ptr = ptr->nextEdge;
    }
}

Graph *sweeping(Graph *graph) 
{
    for (int i = 0; i < MAX; i++)
    {
        if (graph->marked[i] == 0)
        {
            printf ("Garbage Node: %d\n", i);
            Edge *ptr = graph->EdgeList[i];
            Edge *prev = NULL;  
            while (ptr != NULL)
            {
                prev = ptr;
                ptr = ptr->nextEdge;
                deallocate (prev);
            }
            graph->EdgeList[i] = NULL;
        }
    }
    return graph;
}

void referenceCounting(Graph *graph, Node *root)
{
    for (int i = 0; i < MAX; i++)
    {
        graph->references[i] = 0;
    }

    for (int i = 0; i < MAX; i++)
    {
        Edge *ptr = graph->EdgeList[i];
        while (ptr != NULL)
        {
            graph->references[ptr->endPoint]++;
            ptr = ptr->nextEdge;
        }
    }

    while (root != NULL)
    {
        graph->references[root->data]++;
        root = root->next;
    }
    
}

void deleteGraphNodes(Graph *graph, Vertex v)
{
    
    Edge *ptr = graph->EdgeList[v];
    while (ptr != NULL)
    {
        graph->references[ptr->endPoint]--;
        if (graph->references[ptr->endPoint] == 0)
        {
            deleteGraphNodes (graph, ptr->endPoint);
        }
        ptr = ptr->nextEdge;
    }

    Edge *nextptr = graph->EdgeList[v];
    Edge *prev = NULL;  
    while (nextptr != NULL)
    {
        prev = nextptr;
        nextptr = nextptr->nextEdge;
        deallocate (prev);
    }
    graph->EdgeList[v] = NULL;
}

Graph *newDelete(Graph *graph, Node *root)
{
    graph->references[root->data]--;
    if (graph->references[root->data] == 0)
    {
        deleteGraphNodes(graph, root->data); //completely delete
    }
    deallocate (root);
    return graph;
}

int main()
{
    freeList->next = NULL;  // Set next to NULL
    freeList->size = 10000; // Set size to the total size of the memory array
    
    Graph *graph = (Graph *)allocate (sizeof(Graph));
    initialaize(graph);
    Vertex v1, v2;
    scanf ("%d %d", &v1, &v2);
    while (v1 >= 0 && v2 >= 0)
    {
        insertInGraph(graph, v1, v2);
        scanf ("%d %d", &v1, &v2);
    }
    printList (graph);
    Vertex visited[MAX];

    //Once the graph is formed, allocate vertices to the roots
    //Search for the roots, and allocate
    for (int i = 0; i < MAX; i++)
    {
        visited[i] = 0;
    }

    Node *rootPtr = (Node *)allocate(sizeof(Node));
    rootPtr->next = (Node *)allocate(sizeof(Node));
    rootPtr->next->next = NULL;

    rootPtr->data = 1;
    rootPtr->GraphNode = searchForVertex(graph, rootPtr->data, graph->EdgeList[0], visited);

    for (int i = 0; i < MAX; i++)
    {
        visited[i] = 0;
    }
    rootPtr->next->data = 2;
    rootPtr->next->GraphNode = searchForVertex(graph, rootPtr->next->data, graph->EdgeList[0], visited);

    //Marking Phase
    Node *ptr = rootPtr;
    while (ptr != NULL)
    {
        marking (graph, ptr->GraphNode);
        printf("\n");
        ptr = ptr->next;
    }
    
    //sweeping Phase
    graph = sweeping(graph);
    for (int i = 0; i < MAX; i++)
    {
        visited[i] = 0;
    }

    printf ("After Mark and Sweep\n");
    printList (graph);

    // printAdjacencyMatrix(graph);

    // Now reference counting

    printf ("After reference counting\n");
    referenceCounting (graph, rootPtr);
    //a new delete/deallocate fucntion to implement reference Counting garbage collection

    graph = newDelete(graph, rootPtr);

    printList(graph);
    return 0;
}