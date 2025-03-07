#include <SDL.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 900
#define SCREEN_SCALE 50

#define ALPHA 0.010
#define BETA 0.005
#define GAMMA 0.030

#define SCREEN_SHIFT ((Vec3){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, 0})
#define SCREEN_SHIFT_OPP                                                       \
  ((Vec3){-SCREEN_WIDTH / 2.0f, -SCREEN_HEIGHT / 2.0f, 0})

typedef struct Vec3_s {
  float x, y, z;
} Vec3;

typedef struct Matrix_s {
  size_t rows, columns;
  float **data;
} Matrix;

typedef struct Cube_x {
  Vec3 corners[8];
} Cube;

Vec3 vec3_scale(const Vec3 a, const float scalar) {
  return (Vec3){a.x * scalar, a.y * scalar, a.z * scalar};
}

Vec3 vec3_add(const Vec3 a, const Vec3 b) {
  return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

// Matrix operations prototypes
Matrix *create_matrix(const int rows, const int columns);
Matrix *create_general_rotation_matrix();
Matrix *dot_matrix_matrix(const Matrix *a, const Matrix *b);
void print_matrix(const Matrix *matrix);
void free_matrix(Matrix *matrix);
Matrix *vec3_to_matrix(const Vec3 v);
Vec3 transform_matrix_vec3(const Matrix *a, const Vec3 v);
// Matrix operations prototypes

Matrix *create_matrix(const int rows, const int columns) {
  Matrix *pmatrix = malloc(sizeof(Matrix));
  pmatrix->rows = rows;
  pmatrix->columns = columns;
  pmatrix->data = calloc(sizeof(float *), rows);
  for (size_t i = 0; i < rows; i++) {
    pmatrix->data[i] = calloc(sizeof(float), columns);
  }
  return pmatrix;
}

Matrix *create_general_rotation_matrix() {
  Matrix *rotX = create_matrix(3, 3);
  // row 1
  rotX->data[0][0] = 1;
  rotX->data[0][1] = 0;
  rotX->data[0][2] = 0;
  // row 2
  rotX->data[1][0] = 0;
  rotX->data[1][1] = cos(ALPHA);
  rotX->data[1][2] = -sin(ALPHA);
  // row 3
  rotX->data[2][0] = 0;
  rotX->data[2][1] = sin(ALPHA);
  rotX->data[2][2] = cos(ALPHA);

  Matrix *rotY = create_matrix(3, 3);
  // row 1
  rotY->data[0][0] = cos(BETA);
  rotY->data[0][1] = 0;
  rotY->data[0][2] = sin(BETA);
  // row 2
  rotY->data[1][0] = 0;
  rotY->data[1][1] = 1;
  rotY->data[1][2] = 0;
  // row 3
  rotY->data[2][0] = -sin(BETA);
  rotY->data[2][1] = 0;
  rotY->data[2][2] = cos(BETA);

  Matrix *rotZ = create_matrix(3, 3);
  // row 1
  rotZ->data[0][0] = cos(GAMMA);
  rotZ->data[0][1] = -sin(GAMMA);
  rotZ->data[0][2] = 0;
  // row 2
  rotZ->data[1][0] = sin(GAMMA);
  rotZ->data[1][1] = cos(GAMMA);
  rotZ->data[1][2] = 0;
  // row 3
  rotZ->data[2][0] = 0;
  rotZ->data[2][1] = 0;
  rotZ->data[2][2] = 1;

  return dot_matrix_matrix(rotZ, dot_matrix_matrix(rotY, rotX));
}

void print_matrix(const Matrix *matrix) {
  printf("[ \n");
  for (size_t i = 0; i < matrix->rows; i++) {
    printf("[");
    for (size_t a = 0; a < matrix->columns; a++) {
      if (a == (matrix->columns - 1)) {
        printf("%f", matrix->data[i][a]);
      } else {
        printf("%f, ", matrix->data[i][a]);
      }
    }
    printf("],\n");
  }
  printf("]\n");
}

void free_matrix(Matrix *matrix) {
  for (size_t i = 0; i < matrix->rows; i++) {
    free(matrix->data[i]);
  }
  free(matrix->data);
  free(matrix);
}

Matrix *dot_matrix_matrix(const Matrix *a, const Matrix *b) {
  assert(a->columns == b->rows && "a.colums != b.rows");

  Matrix *result = create_matrix(a->rows, b->columns);
  for (size_t i = 0; i < a->rows; i++) {
    for (size_t j = 0; j < a->columns; j++) {
      for (size_t k = 0; k < b->rows; k++) {
        result->data[i][j] += a->data[i][k] * b->data[k][j];
      }
    }
  }

  return result;
}

Vec3 transform_matrix_vec3(const Matrix *a, const Vec3 v) {
  Matrix *point = vec3_to_matrix(v);

  Matrix *result = dot_matrix_matrix(a, point);
  return (Vec3){result->data[0][0], result->data[1][0], result->data[2][0]};
}

Matrix *vec3_to_matrix(const Vec3 v) {
  Matrix *result = create_matrix(3, 1);

  result->data[0][0] = v.x;
  result->data[1][0] = v.y;
  result->data[2][0] = v.z;

  return result;
}

void draw_line_from_vec3(SDL_Renderer *renderer, const Vec3 start,
                         const Vec3 end) {
  SDL_RenderDrawLineF(renderer, start.x, start.y, end.x, end.y);
}

Cube init_cube(const float x, const float y, const float z, const float edge_length) {
  Cube cube;
  for (int i = 0; i < 2; i++) {
    cube.corners[i * 4 + 0] = (Vec3){
        x - edge_length,
        y,
        z - (edge_length * i),
    };
    cube.corners[i * 4 + 1] = (Vec3){
        x,
        y,
        z - (edge_length * i),
    };
    cube.corners[i * 4 + 2] = (Vec3){
        x,
        y - edge_length,
        z - (edge_length * i),
    };
    cube.corners[i * 4 + 3] = (Vec3){
        x - edge_length,
        y - edge_length,
        z - (edge_length * i),
    };
  }
  return cube;
}

int main(void) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

  SDL_Window *window = SDL_CreateWindow(
      "Cube Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (!window) {
    fprintf(stderr, "SDL_Window couldn't be created\n");
    goto cleanup;
    return 1;
  }

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    fprintf(stderr, "SDL_Renderer couldn't be created\n");
    goto cleanup;
    return 1;
  }
  // Initialization

  Matrix *rotationXYZ = create_general_rotation_matrix();

  Cube cube = init_cube(5, 5, 5, 10);
  for (size_t i = 0; i < 8; i++) {
    cube.corners[i] =
        vec3_add(SCREEN_SHIFT, vec3_scale(cube.corners[i], SCREEN_SCALE));
  }

  SDL_Event event;
  bool quit = false;
  while (!quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = true;
      }
    }

    for (size_t i = 0; i < 8; i++) {
      cube.corners[i] = vec3_add(cube.corners[i], SCREEN_SHIFT_OPP);
      cube.corners[i] = transform_matrix_vec3(rotationXYZ, cube.corners[i]);
      cube.corners[i] = vec3_add(cube.corners[i], SCREEN_SHIFT);
    }

    // Rendering
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (size_t i = 0; i < 4; i++) {
      draw_line_from_vec3(renderer, cube.corners[i], cube.corners[(i + 1) % 4]);
      draw_line_from_vec3(renderer, cube.corners[i + 4],
                          cube.corners[((i + 1) % 4) + 4]);
      draw_line_from_vec3(renderer, cube.corners[i], cube.corners[i + 4]);
    }

    SDL_RenderPresent(renderer);
  }

cleanup:
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  return 0;
}
