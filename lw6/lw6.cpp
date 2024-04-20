#include <iostream>
#include <omp.h>

//task 1
int N = 1234567;

int main() {
    double pi = 0.0;
    double startTime;
    double endTime;
    startTime = omp_get_wtime();

    for (int i = 0; i < N; i++) {
        pi += (i % 2 == 0 ? 1.0 : -1.0) / (2 * i + 1);
    }
    pi *= 4;
    endTime = omp_get_wtime();
    double duration = endTime - startTime;
    std::cout << "sync-for: " << pi << ", time: " << duration << " s" << std::endl;

    //-----//
    pi = 0.0;
    startTime = omp_get_wtime();
#pragma omp parallel for
    for (int i = 0; i < N; i++) {
        pi += (i % 2 == 0 ? 1.0 : -1.0) / (2 * i + 1);
    }
    pi *= 4;
    endTime = omp_get_wtime();
    duration = endTime - startTime;
    std::cout << "parallel-for " << pi << ", time: " << duration << " s" << std::endl;

    //-----//
    pi = 0.0;
    startTime = omp_get_wtime();
#pragma omp parallel for
    for (int i = 0; i < N; i++) {
#pragma omp atomic
        pi += (i % 2 == 0 ? 1.0 : -1.0) / (2 * i + 1);
    }
    pi *= 4;
    endTime = omp_get_wtime();
    duration = endTime - startTime;
    std::cout << "parallel-for & atomic: " << pi << ", time: " << duration << " s" << std::endl;

    //-----//
    pi = 0.0;
    startTime = omp_get_wtime();
#pragma omp parallel for reduction(+:pi)
    for (int i = 0; i < N; i++) {
        pi += (i % 2 == 0 ? 1.0 : -1.0) / (2 * i + 1);
    }
    pi *= 4;
    endTime = omp_get_wtime();
    duration = endTime - startTime;
    std::cout << "reduction: " << pi << ", time: " << duration << " s" << std::endl;

    return 0;
}

//task 2
// разница между private(x), firstprivate(x), lastprivate(x)

//int main()
//{
//    int x = 44;
//#pragma omp parallel for private(x)
//    for(int i = 0; i <= 10; i++){
//        x=i;
//        printf("Thread number: %d x: %d\n",omp_get_thread_num(),x);
//    }
//    printf("x is %d\n", x);
//}

// private(x) - каждый поток получает свою собственную копию переменной x,
// исходное значение x не копируется в эти приватные переменные.
// в примере выше x будет инициализирован в каждом потоке, но начальное значение 44 не будет скопировано.

// firstprivate(x) - как и private(x), каждый поток получает свою собственную
// копию переменной x, но в этом случае исходное значение x копируется в каждую приватную переменную.
// то есть, если мы используем firstprivate(x), каждый поток начинает со значением x равным 44.

// lastprivate(x): Это комбинация private и дополнительного поведения:
// после окончания параллельного блока значение x в основном потоке будет равно
// значению x в последнем потоке, который выполнил итерацию цикла. это применяется в том случае,
// когда нужно сохранить результат последней итерации.
// когда нужно сохранить результат последней итерации.