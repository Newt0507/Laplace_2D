#pragma warning (disable : 4996)

#include "mpi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>


/**
 * Hiển thị ma trận đã cho với các tham số được sắp xếp theo thứ tự tăng dần
 */
void print_matrix(int me, float* tab, int nb_rows, int nb_cols)
{
    printf("\n \n Matrix printed by me: %d \n \n", me);
    for (int i = 0; i < nb_rows; i++)
    {
        for (int j = 0; j < nb_cols; j++)
        {
            // Hiển thị ma trận
            printf(" %.2f", *(tab + j + i * nb_cols));

            // Dạng "%d" để test khi không dùng Phương trình Laplace
            //printf(" %d",(int) *(tab + j + i * nb_cols));
        }
        printf("\n");
    }
}


/**
 * Hiển thị ma trận đã cho với các tham số được sắp xếp theo thứ tự giảm dần
 */
void print_matrix_reverse(int me, float* tab, int nb_rows, int nb_cols)
{
    printf("\n\n  Matrix printed by me: %d \n", me);
    for (int i = nb_rows - 1; i >= 0; i--)
    {
        for (int j = 0; j < nb_cols; j++)
        {
            // Hiển thị ma trận
            printf(" %.2f", *(tab + j + i * nb_cols));

            // Dạng "%d" để test khi không dùng Phương trình Laplace
            //printf(" %d",(int) *(tab + j + i * nb_cols));
        }
        printf("\n");
    }
}


/**
 * Cập nhật giá trị của các hàng liền kề của tất cả các ma trận cục bộ
 */
void update_rows(float* local_tab, int Nlocal_rows, int Nlocal_cols, int NPROC, int me, int NBCUTS)
{
    MPI_Status status;
    int msg_tag1 = 1;
    int msg_tag2 = 2;

    /*
     *  1. Truyền tải hàng dữ liệu QUAN TRỌNG đầu tiên để làm mới hàng LIỀN KỀ cuối cùng của bộ xử lý phía bên dưới me
     *      Message TAG = 1
     */
    if (me > NBCUTS - 1) // Nếu me đang không ở hàng đầu tiên của bộ xử lý
        // Gửi hàng thứ 2 của tab cục bộ tới bộ xử lý phía trên me
        MPI_Send(local_tab + Nlocal_cols + 1, Nlocal_cols - 2, MPI_FLOAT, me - NBCUTS, msg_tag1, MPI_COMM_WORLD); 

    if (me < NPROC - NBCUTS) // Nếu me đang không ở hàng cuối cùng của bộ xử lý
        // Sau khi nhận được thông điệp với TAG = 1, cập nhật lại hàng cuối cùng của tab
        MPI_Recv(local_tab + 1 + (Nlocal_rows - 1) * Nlocal_cols, Nlocal_cols - 2, MPI_FLOAT, me + NBCUTS, msg_tag1, MPI_COMM_WORLD, &status); 

    /*
     *  2. Truyền tải hàng dữ liệu QUAN TRỌNG đầu tiên để làm mới hàng LIỀN KỀ cuối cùng của bộ xử lý phía bên dưới me
     *      Message TAG = 2
     */
    if (me < NPROC - NBCUTS) // Nếu me đang không ở hàng cuối cùng của bộ xử lý
        // Gửi hàng thứ 2 của tab cục bộ tới bộ xử lý với index hàng đầu tiên: local_tab+1+(Nlocal_rows-2)*Nlocal_cols)
        MPI_Send(local_tab + 1 + (Nlocal_rows - 2) * Nlocal_cols, Nlocal_cols - 2, MPI_FLOAT, me + NBCUTS, msg_tag2, MPI_COMM_WORLD); 

    if (me > NBCUTS - 1) // Nếu me đang không ở hàng đầu tiên của bộ xử lý
        // Sau khi nhận được thông điệp với TAG = 2, cập nhật lại hàng đầu tiên của tab cục bộ
        MPI_Recv(local_tab + 1, Nlocal_cols - 2, MPI_FLOAT, me - NBCUTS, msg_tag2, MPI_COMM_WORLD, &status); 
}


/**
 * Cập nhật giá trị của các cột liền kề của tất cả các ma trận cục bộ
 */
void update_cols(float* local_tab, int Nlocal_rows, int Nlocal_cols, int NPROC, int me, int NBCUTS)
{
    MPI_Status status;
    int msg_tag3 = 3;
    int msg_tag4 = 4;

    MPI_Datatype column;
    MPI_Type_vector(Nlocal_rows - 2, 1, Nlocal_cols, MPI_FLOAT, &column); // định nghĩa
    MPI_Type_commit(&column);

    /*
     *    3. Truyền tải cột dữ liệu QUAN TRỌNG cuối cùng để làm mới cột LIỀN KỀ đầu tiên của bộ xử lý tiếp theo nằm bên phải
     *      Message TAG = 3
     */
    if (me % NBCUTS != NBCUTS - 1)// Nếu me không là một phần của cột cuối cùng trong bộ xử lý
        // Gửi cột thứ 2 của tab cục bộ tới bộ xử lý tiếp theo với index cột đầu tiên: local_tab+Nlocal_cols*2-2
        MPI_Send(local_tab + Nlocal_cols * 2 - 2, 1, column, me + 1, msg_tag3, MPI_COMM_WORLD); 

    if (me % NBCUTS != 0) // Nếu me không là một phần của cột đầu tiên trong bộ xử lý
        // Sau khi nhận được thông điệp với TAG = 3, cập nhật lại cột đầu tiên của tab cục bộ
        MPI_Recv(local_tab + Nlocal_cols, 1, column, me - 1, msg_tag3, MPI_COMM_WORLD, &status); 

    /*
     *    4. Truyền tải cột dữ liệu QUAN TRỌNG đầu tiên để làm mới cột LIỀN KỀ cuối cùng của bộ xử lý trước đó nằm bên trái
     *      Message TAG = 4
     */
    if (me % NBCUTS != 0) // Nếu me không là một phần của cột đầu tiên trong bộ xử lý
        // Gửi cột thứ 2 của tab cục bộ tới bộ xử lý trước đó với index cột đầu tiên: local_tab+Nlocal_cols+1
        MPI_Send(local_tab + Nlocal_cols + 1, 1, column, me - 1, msg_tag4, MPI_COMM_WORLD);

    if (me % NBCUTS != NBCUTS - 1) // Nếu me không là một phần của cột cuối cùng trong bộ xử lý
        // Sau khi nhận được thông điệp với TAG = 4, cập nhật lại hàng cuối cùng của tab cục bộ
        MPI_Recv(local_tab + Nlocal_cols * 2 - 1, 1, column, me + 1, msg_tag4, MPI_COMM_WORLD, &status);  
}


/**
 * Cập nhật giá trị của các hàng và cột liền kề của tất cả các ma trận cục bộ
 */
void update_matrix(float* local_tab, int Nlocal_rows, int Nlocal_cols, int NPROC, int me, int NBCUTS)
{
    update_rows(local_tab, Nlocal_rows, Nlocal_cols, NPROC, me, NBCUTS);
    update_cols(local_tab, Nlocal_rows, Nlocal_cols, NPROC, me, NBCUTS);
}


/**
 * Thực thi phương trình Laplace
 */
void laplace(float* local_tab, int Nlocal_rows, int Nlocal_cols, int NPROC, int me, int NBCUTS)
{
    // tab tạm thời lưu trữ giá trị mới của local_tab
    float* new_tab = (float*)malloc(Nlocal_rows * Nlocal_cols * sizeof(float));
    if (new_tab == NULL) { exit(-1); } // Kiểm tra bộ nhớ đã được phân bố tốt hay chưa?

    double PRECISION = 1.0e-2; // Độ chính xác
    double global_error = +INFINITY;

    int iter_count = 0;

    // Khi lỗi vẫn chưa được tinh toán CHÍNH XÁC, tiếp tục vòng lặp
    while (global_error >= PRECISION) 
    {
        double local_error_sum = 0;
        iter_count++;

        for (int i = 1; i < Nlocal_rows - 1; i++)
        {
            for (int j = 1; j < Nlocal_cols - 1; j++)
            {
                float top_neighbor = *(local_tab + j + (i - 1) * Nlocal_cols);
                float bottom_neighbor = *(local_tab + j + (i + 1) * Nlocal_cols);
                float left_neighbor = *(local_tab + (j - 1) + i * Nlocal_cols);
                float right_neighbor = *(local_tab + (j + 1) + i * Nlocal_cols);

                // Công thức tính của Phương trình Laplace
                *(new_tab + j + i * Nlocal_cols) = 0.25 * (bottom_neighbor + top_neighbor + left_neighbor + right_neighbor); 

                // Tính toán lỗi và thêm vào local_error_sum
                local_error_sum += (*(new_tab + j + i * Nlocal_cols) - *(local_tab + j + i * Nlocal_cols))
                    * (*(new_tab + j + i * Nlocal_cols) - *(local_tab + j + i * Nlocal_cols));
            }
        }

        //  Thay thế giá trị của local_tab bằng giá trị new_tab
        for (int i = 1; i < Nlocal_rows - 1; i++)
        {
            for (int j = 1; j < Nlocal_cols - 1; j++)
            {
                *(local_tab + j + i * Nlocal_cols) = *(new_tab + j + i * Nlocal_cols);
            }
        }

        update_matrix(local_tab, Nlocal_rows, Nlocal_cols, NPROC, me, NBCUTS);
        double global_error_sum = 0;

        // Tổng hợp lỗi của tất cả các bộ xử lý và đưa kết quả vào global_error_sum
        MPI_Allreduce(&local_error_sum, &global_error_sum, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD); 

        global_error = sqrt(global_error_sum);
        if (me == 0)
        {
            printf(" Iteration %d - error = %e\n", iter_count, global_error);
        }
    }
    free(new_tab);
}


/**
 * Gather all the local matrices data from the processors and print the whole reconstructed matrix
 * Thu thập tất cả dữ liệu ma trận cục bộ từ bộ xử lý và hiển thị ma trận đã được xây dựng lại
 */
void print_and_save_final_matrix(char filename[], int me, float* local_tab, int Nlocal_rows, int Nlocal_cols, int N, int NPROC, int NBCUTS)
{
    int i, j;
    MPI_Status status;
    MPI_Request req;

    MPI_Datatype mysubarray; // Tạo một kiểu dữ liệu cho một mảng con của một mảng đa chiều -> Sử dụng trên local_tab để lấy các giá trị QUAN TRỌNG
    int starts[2] = { 1, 1 }; // Tọa độ phần tử đầu tiên trong local_tab là (1, 1)
    int subsizes[2] = { Nlocal_rows - 2, Nlocal_cols - 2 }; // Kích thước của mảng con để lấy giá trị
    int bigsizes[2] = { Nlocal_rows, Nlocal_cols }; // Kích thước của mảng local_tab muốn lấy dữ liệu cho mảng con

    MPI_Type_create_subarray(2, bigsizes, subsizes, starts, MPI_ORDER_C, MPI_FLOAT, &mysubarray); // tạo kiểu dữ liệu mới với 2 là số chiều
    MPI_Type_commit(&mysubarray); // commit dữ liệu

    MPI_Isend(local_tab, 1, mysubarray, 0, me, MPI_COMM_WORLD, &req); // Gửi các giá trị QUAN TRỌNG của local_tav (sử dụng kiểu dữ liệu mới) tới bộ xử lý 0

    MPI_Type_free(&mysubarray);
    MPI_Barrier(MPI_COMM_WORLD);

    int nb_subdata = (Nlocal_rows - 2) * (Nlocal_cols - 2); // số lượng dữ liệu quan trọng (không có dữ liệu liền kề) trong local_tab

    if (me == 0) // bộ xử lý 0 chịu trách nhiệm thu thập tất cả dữ liệu
    {
        /*
            Bước 1 : Nhận dữ liệu từ tất cả bộ xử lý
            recv_matrix có dạng : row 0 - dữ liệu nhận từ bộ xử lý 0, row 1 - dữ liệu nhận từ bộ xử lý 1, vân vân...
            0 0 0 0 0 0 0 0 0 0 -> dữ liệu từ bộ xử lý 0
            1 1 1 1 1 1 1 1 1 1 -> dữ liệu từ bộ xử lý 1
            2 2 2 2 2 2 2 2 2 2 -> dữ liệu từ bộ xử lý 2
            3 3 3 3 3 3 3 3 3 3 -> dữ liệu từ bộ xử lý 3
        */
        float* recv_matrix = (float*)malloc(N * N * sizeof(float));
        if (recv_matrix == NULL) { exit(-1); } // Kiểm tra xem bộ nhớ đã được phân bổ tốt chưa

        // Đối với tất cả các bộ xử lý, bộ xử lý 0 lưu trữ các giá trị nhận được trong 1 hàng recv_matrix
        for (i = 0; i < NPROC; i++)
        {
            MPI_Recv(recv_matrix + i * nb_subdata, nb_subdata, MPI_FLOAT, i, i, MPI_COMM_WORLD, &status);
        }

        printf("\n  Recv data is :");
        print_matrix(me, recv_matrix, NPROC, nb_subdata);
        printf("\n ------------------------------- \n");

        /*
            Bươc 2 : Sắp xếp lại các hàng theo mô đun xếp hạng bộ xử lý của chúng
            0 0 0 0 0 0 0 0 0 0 -> dữ liệu từ bộ xử lý 0 => mô đun 0
            2 2 2 2 2 2 2 2 2 2 -> dữ liệu từ bộ xử lý 2 => mô đun 0
            1 1 1 1 1 1 1 1 1 1 -> dữ liệu từ bộ xử lý 1 => mô đun 1
            3 3 3 3 3 3 3 3 3 3 -> dữ liệu từ bộ xử lý 3 => mô đun 1
        */
        float* ordered_matrix = (float*)malloc(N * N * sizeof(float));
        if (ordered_matrix == NULL) { exit(-1); } // Kiểm tra xem bộ nhớ đã được phân bổ tốt chưa
        int pivot_inter = 0;
        int modulo_result = 0;
        for (int modulo_result = 0; modulo_result < NBCUTS; modulo_result++)
        {
            for (int i = 0; i < NPROC; i++)
            {
                if (i % NBCUTS == modulo_result)
                {
                    for (int j = 0; j < nb_subdata; j++)
                    {
                        *(ordered_matrix + j + pivot_inter * nb_subdata) = *(recv_matrix + j + i * nb_subdata);
                    }
                    pivot_inter = pivot_inter + 1;
                }
            }
        }

        printf("  Intermediate data is :");
        print_matrix(me, ordered_matrix, NPROC, nb_subdata);
        printf("\n ------------------------------- \n");


        /*
            Bước 3: Sắp xếp theo khối để tạo thành ma trận cuối cùng
            0 0 0 0 0 1 1 1 1 1                                     2 2 2 2 2 3 3 3 3 3
            0 0 0 0 0 1 1 1 1 1    ------------------------->       2 2 2 2 2 3 3 3 3 3
            2 2 2 2 2 3 3 3 3 3      print_matrix_reverse()         0 0 0 0 0 1 1 1 1 1
            2 2 2 2 2 3 3 3 3 3                                     0 0 0 0 0 1 1 1 1 1
        */
        float* final_matrix = (float*)malloc(N * N * sizeof(float));
        if (final_matrix == NULL) { exit(-1); } // Kiểm tra xem bộ nhớ đã được phân bổ tốt chưa
        int offset = 0;

        for (int vertical_step = 0; vertical_step < NBCUTS; vertical_step++)
        {
            for (int i = 0; i < N; i++)
            {
                for (int j = (Nlocal_cols - 2) * vertical_step; j < ((Nlocal_cols - 2) * vertical_step) + (Nlocal_cols - 2); j++)
                {
                    *(final_matrix + j + i * N) = *(ordered_matrix + offset);
                    offset = offset + 1;
                }
            }
        }

        // Hiển thị ma trận cuối cùng
        printf("  Final solution is:");
        print_matrix_reverse(me, final_matrix, N, N);
        printf("\n ------------------------------- \n");

        // Lưu ma trận cuối cùng vào file với thứ tự đảo ngược
        FILE* f;
        if (me == 0)
        {
            if ((f = fopen(filename, "w")) == NULL) { perror("matrix_save: fopen "); }
            for (int i = N - 1; i >= 0; i--)

            {
                for (int j = 0; j < N; j++)
                {
                    // Hiển thị ma trận
                    fprintf(f, "%f ", *(final_matrix + j + i * N));

                    // Dạng "%d" để test khi không dùng Phương trình Laplace
                    //fprintf (f, "%d ", (int) *(final_matrix + j + i*N) );
                }
                fprintf(f, "\n");
            }
            fclose(f);
        }

        free(recv_matrix);
        free(ordered_matrix);
        free(final_matrix);
    }
}


/**
 * Khởi tạo ma trận cục bộ: tất cả các giá trị được đặt theo thứ hạng của bộ xử lý, ngoại trừ các giá trị liền kề được mặc định là -1
 * Ví dụ: Ma trận có thứ hạng là 0 có ma trận cục bộ 5x5 thì kết quả là:
 * -1 -1 -1 -1 -1
   -1  0  0  0 -1
   -1  0  0  0 -1
   -1  0  0  0 -1
   -1 -1 -1 -1 -1
 */
void initialize_local_matrix(int me, int NPROC, int nb_cols, float* local_tab, int nb_rows)
{
    //int count = 0; // Sử dụng để Debug
    for (int i = 0; i < nb_rows; i++)
    {
        for (int j = 0; j < nb_cols; j++)
        {
            if ((i == 0) || (i == nb_rows - 1))
            {
                *(local_tab + j + i * nb_cols) = -1;
            }
            else if ((j == 0) || (j == nb_cols - 1))
            {
                *(local_tab + j + i * nb_cols) = -1;
            }
            else
            {
                *(local_tab + j + i * nb_cols) = me;
                //count++;
            }
        }
    }
}


/**
 * Kiểm tra xem số đó có phải là số chính phương hay không
 * Vấn đề có thể xảy ra đối với một số lượng rất lớn
 */
int is_perfect_square(int number)
{
    int intResult;
    float floatResult;

    floatResult = sqrt((double)number);
    intResult = floatResult;

    if (intResult == floatResult) return 1;
    else return 0;
}


/**
 * Hàm main
 */
int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Argument missing. Usage: mpirun -np [number of processors] ./my_exe [N square matrix dimension]\nexample: mpirun -np 9 ./my_exe 12\n");
        exit(-1);
    }

    MPI_Init(&argc, &argv);
    int NPROC, me; // NPROC: số lượng bộ xử lý, me: thứ hạng của bộ xử lý thực tế
    double start_time, max_time, min_time, avg_time, local_time;
    int N = atoi(argv[1]); // Khời tạo ma trận vuông

    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &NPROC);
    MPI_Barrier(MPI_COMM_WORLD);  // Đồng bộ tất cả các quy trình

    start_time = MPI_Wtime();  // Lấy thời gian trước phiên làm việc

    int NBCUTS; // Ma trận ban đầu bị cắt đi bao nhiều hàng và cột
    if (is_perfect_square(NPROC) == 1) // Nếu số lượng bộ xử lý là số chính phương, tính căn bậc 2
    {
        NBCUTS = sqrt(NPROC);
    }
    else
    {
        printf("ERROR: In this version, number of cuts must be the same for both columns and rows. Please choose a perfect square number of processors (as 4 or 9 for example.).\n");
        MPI_Finalize();
        exit(-1);
    }

    const int NBLOCK = N / NBCUTS;  // số hàng và cột QUAN TRỌNG trong khối
    if (N != NBCUTS * NBLOCK)
    {
        printf("ERROR: imcompatible number of processors and matrix size. Multiplication between number of cuts (NBCUTS - how many parts rows and columns of the original matrix is cut) and number of significant rows and columns in a block should be equal to N the number of rows and columns of the original matrix. We expect N = NBCUTS x NBLOCK, but we have %d = %d x %d\n", N, NBCUTS, NBLOCK);
        MPI_Finalize();
        exit(-1);
    }

    int Nlocal = NBLOCK + 2; // Số hàng thực tế của ma trận cục bộ -> thêm 2 "hàng xóm"
    float* local_tab = NULL; // Tất cả các bộ xử lý có ma trận cục bộ của riêng chúng chứa các giá trị ma trận gốc mà chúng chịu trách nhiệm + giá trị liền kề
    local_tab = (float*)malloc(sizeof(float) * Nlocal * Nlocal);
    if (local_tab == NULL) { exit(-1); } // Kiểm tra xem bộ nhớ đã được phân bổ tốt chưa

    // TÍNH TOÁN VÀ ĐIỀN MA TRẬN
    initialize_local_matrix(me, NPROC, Nlocal, local_tab, Nlocal);
    update_matrix(local_tab, Nlocal, Nlocal, NPROC, me, NBCUTS); // cập nhật đầu tiên của các giá trị "hàng xóm"
    
    // Thực thi phương trình Laplace
    laplace(local_tab, Nlocal, Nlocal, NPROC, me, NBCUTS);


    // ĐÁNH GIÁ HIỆU SUẤT
    local_time = MPI_Wtime() - start_time;  // Lấy thời gian ngay sau phiên làm việc

    MPI_Reduce(&local_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_time, &min_time, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_time, &avg_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    // Hiển thị các ma trận cục bộ (không để đánh giá hiệu suất)
    for (int i = 0; i < NPROC; i++)
    {
        if (me == i)
        {
            print_matrix(i, local_tab, Nlocal, Nlocal);
        }
        MPI_Barrier(MPI_COMM_WORLD);  // Đồng bộ quá tất cả các quy trình để tránh "chồng chéo" trong quá trình in
    }

    if (me == 0)
    {
        avg_time /= NPROC;
        printf("\n  Min: %lf seconds.  Max: %lf seconds.  Avg:  %lf seconds.\n", min_time, max_time, avg_time);
    }

    print_and_save_final_matrix("result_laplace_2D.txt", me, local_tab, Nlocal, Nlocal, N, NPROC, NBCUTS);
        
    MPI_Finalize();
    free(local_tab);
    return 0;
}
