#include <SDL2/SDL_timer.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <omp.h>
#include <stdbool.h>

// Structure to hold thread arguments to process canvas and initialize canvas
typedef struct {
    bool ** canvaOld;
    bool ** canva;
    int apa;
    int startRow;
    int endRow;
} ThreadArgs;


int countNeighbors(bool ** canva, int x, int y, int apa){
    int count = 0;
    
    count += ((x != (apa - 1)) && (canva[x + 1][y] == 1 ));
    count += ((x != (apa - 1)) && (y != (apa - 1))) && (canva[x + 1][y + 1] == 1);
    count += ((x != (apa - 1)) && (y != 0)) && (canva[x + 1][y - 1] == 1);
    count += (x != 0) && (canva[x - 1][y] == 1);
    count += ((x != 0) && (y != (apa - 1))) && (canva[x - 1][y + 1] == 1);
    count += ((x != 0) && (y != 0)) && (canva[x - 1][y - 1] == 1);
    count += ((y != (apa - 1))) && (canva[x][y + 1] == 1);
    count += ((y != 0)) && (canva[x][y - 1] == 1);
    return count;
    
}

void* processCanvasThread(void* arg) {
    ThreadArgs* args = (ThreadArgs*) arg;

    bool ** canva = args->canva;
    bool ** canvaOld = args->canvaOld;
    int apa = args->apa;
    int startRow = args->startRow;
    int endRow = args->endRow;
    int neighbors = 0;

    for (int i = startRow; i < endRow; i++) {
        for (int j = 0; j < apa; j++) {
            neighbors = countNeighbors(canvaOld, i, j, apa);
            canva[i][j] = ((canvaOld[i][j] == 1) && ((neighbors == 2) || (neighbors == 3))) + ((canvaOld[i][j] == 0) && ((neighbors == 3)));
        }
    }
    pthread_exit(NULL);
}

void processCanvas(bool ** canvaOld, bool ** canva, int apa, pthread_t * threads, ThreadArgs * args, int numThreads) {

    int rowsPerThread = apa / numThreads;
    int startRow = 0;
    
    for (int i = 0; i < numThreads; i++) {
        int endRow = startRow + rowsPerThread;

        if (i == numThreads - 1) {
            endRow = apa;
        }

        args[i].canvaOld = canvaOld;
        args[i].canva = canva;
        args[i].apa = apa;
        args[i].startRow = startRow;
        args[i].endRow = endRow;

        startRow = endRow;
    }
    for(int i = 0; i < numThreads; i++){
        pthread_create(&threads[i], NULL, processCanvasThread, &args[i]);
    }
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }
    
}

void * initializeRows(void * arg){
    ThreadArgs * args = (ThreadArgs *) arg;
    int number = 0;
    int seed = rand();
    bool ** canva = args->canva;
    bool ** canvaOld = args->canvaOld;
    for(int i = args->startRow; i < args->endRow; i++){
        for(int j = 0; j < args->apa; j ++){
          number = rand_r(&seed);
          canva[i][j] = (number % 2 == 0);
          canvaOld[i][j] = (number % 2 == 0);
        }
    }
    pthread_exit(NULL);
}

void initializeCanvas(bool ** canvaOld, bool ** canva, int apa, int numThreads){
    int startRow = 0;
    int rowsPerThread = apa / numThreads;
      
    ThreadArgs * args = malloc(sizeof(ThreadArgs) * numThreads);
    pthread_t * threads = malloc(sizeof(pthread_t) * numThreads);
    
    for( int i = 0; i < numThreads; i ++){
        int endRow = startRow + rowsPerThread;
        if(i == numThreads - 1){
          endRow = apa;
        }
        
        args[i].canva = canva;
        args[i].canvaOld = canvaOld;
        args[i].apa = apa;
        args[i].startRow = startRow;
        args[i].endRow = endRow;
        
        startRow = endRow;
    }

    for(int i = 0; i < numThreads; i ++){
      pthread_create(&threads[i], NULL, initializeRows, &args[i]);
      printf("Foi thread %i\n", i);
    }
    
    for(int i = 0; i < numThreads; i ++){
      pthread_join(threads[i], NULL);
    }
}

int main (){

    srand(time(NULL));

    // apa -> Aumount per axis
    int WIDTH = 1000, HEIGHT = 1000, apa = 50000;
    int numThreads = 32;
    //Initialization of SDL
    /*
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("SDL not initialized.\n");
    }
    else{
        printf("Let's gooo!\n");
    }
    
    //SDL renderer and window
    SDL_Window *window = NULL;
    SDL_Event event;
    SDL_Renderer *render;

    //Creating window
    
    window = SDL_CreateWindow(
        "Game of Life",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH,
        HEIGHT,
        0
    );
    if(window == NULL){
        return 0 ;
    }

    //Creating renderer
    render = SDL_CreateRenderer(window, 0, 0);
    
    //Condition to run the program
    short int run = 1;
    
    //Size per life
    float size = 0;
    size = WIDTH / ((float) apa);
    */
    //Canvas initialization
    double start, end;
    bool **canvas = (bool **) malloc(apa * sizeof(bool *));
    bool **canvasP = (bool **) malloc(apa * sizeof(bool *));

    for(int i = 0; i < apa; i ++){
        canvas[i] = (bool  *) malloc(apa * sizeof(bool ));
        canvasP[i] = (bool  *) malloc(apa * sizeof(bool ));
    }
    
    int aliveCount = 0;
    int deadCount = 0;

    start = omp_get_wtime();
    
    int number = 0;
    
    initializeCanvas(canvas, canvasP, apa, numThreads);
    end = omp_get_wtime();
    printf("Amount per axis: %i\n", apa);
    
    /*
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = size;
    rect.h = size;
    */
    
    
    double frameTimes[100];
    double frameTime = 0;
    double mean = 0;
    short int frameNumber = 0;
  
    int r, g, b;
    float halfApa = apa / 2.0;
    r = 0;
    g = 0;
    b = 0;
    
    pthread_t* threads = (pthread_t*)malloc(numThreads * sizeof(pthread_t));

    // Create thread arguments
    ThreadArgs* args = (ThreadArgs*)malloc(numThreads * sizeof(ThreadArgs));
    int count = 3;

    frameTime = (((double) (end - start)));
    printf("Loading time: %f\n", frameTime);
    while (count > 0)
    {  
        /*
        if(frameNumber == 100){
          frameNumber = 0;
          mean = 0;
          for(int i = 0; i < 100; i++){
            mean += frameTimes[i];
          }
          mean /= 100;
          //printf("\n\nMean frametime: %f\n", mean);
          //printf("Average FPS: %f\n\n\n", 1 / mean);
          //SDL_Delay(2000); 
        }
        
        
        SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
        SDL_RenderClear(render);
        
        while (SDL_PollEvent(&event) != 0)
        {
            if(event.type == SDL_QUIT){
                run = 0;
            }
        }
        
        
        for(int i = 0; i < apa; i ++){
            rect.x = size * i;
            for(int j = 0; j < apa; j ++){
                rect.y = size * j;
                
                if(canvasP[i][j] == 1){
                  r = (1.0 - fabs(((rect.x - (WIDTH / 2.0)) / (WIDTH / 2.0)))) * 255;  
                  g = (1.0 - fabs(((rect.y - (WIDTH / 2.0)) / (WIDTH / 2.0)))) * 255;
                  b = (1.0 - fabs(((((rect.x + rect.y) / 2.0) - (WIDTH / 2.0)) / (WIDTH / 2.0)))) * 255;
                  SDL_SetRenderDrawColor(render, r, g, b, 255);
                  SDL_RenderFillRect(render, &rect);
                }
            }
        }
    
        SDL_RenderPresent(render);
        */
        start = omp_get_wtime();
        processCanvas(canvas, canvasP, apa, threads, args, numThreads);
        bool ** tmp = canvas;
        canvas = canvasP;
        canvasP = tmp;
        end =  omp_get_wtime();
        //Calculate the time taken 
        frameTime = ((double) (end - start));
        printf("Frame took: %f\n", frameTime);
        count--;
        //frameNumber++;
    }
    
    //SDL_DestroyWindow(window);
    //SDL_Quit();

    return 0 ;
}
