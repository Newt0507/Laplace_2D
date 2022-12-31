# Laplace_2D
## CÀI ĐẶT THIẾT LẬP MÔI TRƯỜNG VS C#.NET VỚI MPI 
### Bước 1:  Cài đặt thư viện MPI cho VS 2019 (trên máy đã cài sẵn VS 2019) 
-	Bộ cài đặt gồm 2 file: msmpisetup.exe,  msmpisdk.msi 
-	Tiến hành cài đặt 2 tệp
### Bước 2: Cấu hình MPI trong VS 2019 
#### Bước 2.1: Trong môi trường VS 2019, mở dự án mới 
 ![image](https://user-images.githubusercontent.com/84506206/210128779-29c52775-b4e3-4f43-9677-88671c687b82.png)
 ![image](https://user-images.githubusercontent.com/84506206/210128799-e8357ad1-965e-4922-a0fb-925d4d28327c.png)

#### Bước 2.2: Trong dự án mới, mở cửa sổ properties của dự án 
 ![image](https://user-images.githubusercontent.com/84506206/210128806-17b6c043-ed52-48bc-afd3-9ffe183e18c2.png)
 
-	Thêm vào Additional Include Directories: $(MSMPI_INC);$(MSMPI_INC)\x64
![image](https://user-images.githubusercontent.com/84506206/210128456-5e8aa9bc-5d1d-4adc-a5d5-5445cdcf218b.png)

- Thêm vào Additional Dependencies của Linker/Input thư viện msmpi.lib, lưu ý thêm dấu ‘;’ vào sau msmpi.lib để phân tách với chuỗi khác. 
![image](https://user-images.githubusercontent.com/84506206/210128469-23e95c25-6ef4-44f7-80d6-74c875cbc158.png)

-	Thêm vào mục  Additional Library Directories  của Linker/General chuỗi: $(MSMPI_LIB64)  
![image](https://user-images.githubusercontent.com/84506206/210128476-622c6edb-f278-47cb-b56e-aaee289ae5cd.png)
## KẾT QUẢ CHƯƠNG TRÌNH
#### File Rresult_laplace_2D.txt trong Laplace_2D/Laplace_2D/x64/Debug/

## THỰC THI CHƯƠNG TRÌNH
#### Sau khi Build chương trình ra file .exe -> Chạy file Run_Laplace_2D.bat trong Laplace_2D/Laplace_2D/x64/Debug để hiển thị kết quả
