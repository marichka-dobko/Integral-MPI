//
// Created by maria on 26.05.17.
//

#include <stdio.h>
#include <stdbool.h>
#include <mpi.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <assert.h>
#include <map>
#include <string>
#include <sstream>

double func_calculation(double m, double x1, double x2) {
    double sum1 = 0;
    double sum2 = 0;
    double g;
    for (int i = 1; i <= m; ++i) {
        sum1 += i * cos((i + 1) * x1 + 1);
        sum2 += i * cos((i + 1) * x2 + 1);
    }

    g = -sum1 * sum2;

    return g;
}

double integration(double x0, double x, double y0, double y, double m, double pr) {
    assert (m >= 5);
    double sum = 0;
    for (double i = x0; i <= x; i += pr) {
        for (double j = y0; j <= y; j += pr) {
            sum += func_calculation(m, i + pr / 2.0, j + pr / 2.0) * pr * pr;
        }
    }
    return sum;
}

double thread_integration(double x0, double x, double y0, double y, double m, double pr) {
    auto result = integration(x0, x, y0, y, m, pr);
    return result;
}


int main(int argc, char *argv[]) {
    double abs_er, rel_er, x0, x1, y0, y1;
    double m;
    const long long max_number = 20000000;
    x0 = 0;
    x1 = 3;
    y0 = 0;
    y1 = 1;
    m = 5;
    double pr = 1E-3;

    int commsize, rank, len;
    char procname[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(procname, &len);

    double res = 0;
    double interval_x = (x1 - x0) / commsize;

    double from_to[] = {x0, x0 + interval_x, y0, y1, m, pr};
    if (rank == 0) {
        double start_time = MPI_Wtime();
        for (int i = 1; i < commsize; ++i) {
            MPI_Send(from_to, 6, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
            from_to[0] = from_to[1];
            from_to[1] += interval_x;
        }
#ifdef PRINT_PARTS
        printf("%d: %lld - %lld\n", 0, from_to[0], max_number);
#endif // PRINT_PARTS
        res = thread_integration(from_to[0], from_to[1], from_to[2], from_to[3], from_to[4], from_to[5]);
#ifdef PRINT_PARTS
        printf("Recv from %d: %lld\n", 0, res);
#endif // PRINT_PARTS
        for (int i = 1; i < commsize; ++i) {
            double tr;
            MPI_Recv(&tr, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
#ifdef PRINT_PARTS
            printf("Recv from %d: %lld\n", i, tr);
#endif // PRINT_PARTS

            res += tr;
        }
        printf("Result: %f\n", res);
        printf("Total time: %g \n", MPI_Wtime() - start_time);
    } else {
        double start_time = MPI_Wtime();
        MPI_Recv(from_to, 6, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        res = thread_integration(from_to[0], from_to[1], from_to[2], from_to[3], from_to[4], from_to[5]);
        MPI_Send(&res, 1, MPI_UNSIGNED_LONG, 0, 0, MPI_COMM_WORLD);
        printf("Process %d/%d execution time: %.6f\n", rank, commsize, MPI_Wtime() - start_time);
    }

    MPI_Finalize();
    return 0;
}