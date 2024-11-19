#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void sum_elements_mod13(void *bufin, void *bufout, int *len, MPI_Datatype *datatype) 
{
    int *in = bufin;
    int *out = bufout;

    int x, y;
    for (size_t i = 0; i < *len; i++)
    {
        x = *in;
        y = *out;

        if (x % 13)
            x = 0;
        if (y % 13)
            y = 0;

        *out = x + y;
        in++;
        out++;
    }
}


int main(int argc, char ** argv){

        int myrank, nprocs, len, dest, i;
        char name[MPI_MAX_PROCESSOR_NAME];
        int *buf, *outbuf, *reduce_buf;
        MPI_Status st;

        MPI_Init(&argc, &argv);
        MPI_Op op;
        MPI_Op_create(sum_elements_mod13, 1, &op);
        MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
        MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
        MPI_Get_processor_name(name, &len);

        printf("Hello from processor %s[%d] %d of %d  \n", name, len, myrank, nprocs);

        if (nprocs < 2)
        {
            printf("Too small set of processors!!\n");
            MPI_Finalize();
            return 1;
        }

        if (myrank == 0)
        {
            buf = (int*)malloc(sizeof(int) * (nprocs));

            printf("[%i] Initial array: ", myrank);

            for (i = 0; i < nprocs; i++)
            {
                int random = rand() % 13;

                if (random <= 6)
                    buf[i] = random * 13;
                else 
                    buf[i] = random;

                printf("%i ", buf[i]);
            }

            printf("\n");

        }
        else
            buf = NULL;

        outbuf = (int*)malloc(sizeof(int) * 1);

        MPI_Scatter(buf, 1, MPI_INT, outbuf, nprocs, MPI_INT, 0, MPI_COMM_WORLD);

        printf("[%i] My value: %i\n", myrank, outbuf[0]);

	reduce_buf = (int*)malloc(sizeof(int) * (nprocs));

	MPI_Reduce(outbuf, reduce_buf, nprocs, MPI_INT, op, nprocs - 1, MPI_COMM_WORLD);

	if (myrank == nprocs - 1)
 	    printf("[%i] Result: %i\n", myrank, reduce_buf[0]);

        MPI_Finalize();
        return 0;
}
