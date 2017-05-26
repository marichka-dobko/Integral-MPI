#include <stdio.h>
#include <stdbool.h>
#include <mpi.h>

bool isPrime(unsigned long number){

    if(number < 2)
        return false;
    if(number == 2)
        return true;
    if(number % 2 == 0)
        return false;
    for(unsigned long i=3; (i*i)<=number; i+=2) // i*i -- øâèäøå, í³æ êâàäðàòíèé êîð³íü
    {
        if(number % i == 0 )
            return false;
    }
    return true;

}

//! Àëãîðèòì íàâìèñíå íååôåêòèâíèé --- ùîá âèêîíàííÿ çàäà÷³ òðèâàëî ÿêèéñü ïîì³òíèé ÷àñ
unsigned long printPrime(unsigned long from, unsigned long to)
{
    unsigned long res = 0;
    // x = x + x%2 -- next even nuber for x
    for(unsigned long i=from + (1 - from%2); i<to; i+=2 )
    {
        if(isPrime(i))
        {
            ++res;
        }
    }
    return res;
}

#define PRINT_PARTS

int main(int argc, char *argv[]){
    const long long max_number = 20000000;

    int commsize, rank, len;
    char procname[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(procname, &len);

    long long res = 0;

    long long int from_to[] = {1, max_number/commsize};
    if(rank == 0)
    {
        double start_time = MPI_Wtime();
        for(int i = 1; i<commsize; ++i)
        {

            printf("%d: %lld - %1lld\n", i, from_to[0], from_to[1]);

            MPI_Send(from_to, 2, MPI_UNSIGNED_LONG, i, 0, MPI_COMM_WORLD);
            from_to[0] = from_to[1] + 1;
            from_to[1] = from_to[0] + max_number/commsize - 1;
        }

        printf("%d: %lld - %lld\n", 0, from_to[0], max_number);

        res = printPrime(from_to[0]+1, max_number);

        printf("Recv from %d: %lld\n", 0, res);

        for(int i = 1; i<commsize; ++i)
        {
            long long tr;
            MPI_Recv(&tr, 1, MPI_UNSIGNED_LONG, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Recv from %d: %lld\n", i, tr);
            res += tr;
        }
        printf("Result: %lld\n", res);
        printf("Total time: %g \n",   MPI_Wtime() - start_time);
    }else
    {
        double start_time = MPI_Wtime();
        MPI_Recv(from_to, 2, MPI_UNSIGNED_LONG, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        res = printPrime(from_to[0], from_to[1]);
        MPI_Send(&res, 1, MPI_UNSIGNED_LONG, 0, 0, MPI_COMM_WORLD);
        printf("Process %d/%d execution time: %.6f\n", rank, commsize, MPI_Wtime() - start_time);
    }

    MPI_Finalize();
    return 0;
}