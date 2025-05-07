# Operating System Honored Program Exercise – CO201D HK242

## Sinh viên thực hiện:
- Nguyễn Thiện Minh – 2312097  
- Huỳnh Đức Nhân – 2312420

---

## Mô tả dự án

Trong project này, nhóm em đã hiện thực mô hình định thời **Completely Fair Scheduler (CFS)** dựa trên *Initial Code* được cung cấp trong bài tập lớn.

Việc hiện thực được tích hợp đồng thời với mô hình **Multi-Level Queue (MLQ)** thông qua hai macro:

- `#define CFS_SCHED`
- `#define MLQ_SCHED`  
 
Cấu hình mặc định là **CFS**. Để chuyển sang mô hình **MLQ**, thầy/cô có thể **comment dòng `#define CFS_SCHED`** trong file `os-cfg.h`.

---

## Cách chạy chương trình

File `run.sh` đã được viết để hỗ trợ 3 chế độ chạy:

- `./run.sh make` — dọn sạch và biên dịch lại bằng Makefile  
- `./run.sh run` — chạy toàn bộ các test case liên quan đến `sched`, `paging`, `killall`  
- `./run.sh <tên_testcase>` — chạy riêng một testcase cụ thể

Nhóm đã thêm 3 test case mới: `os_cfs_1`, `os_cfs_2`, `os_cfs_3` để kiểm thử mô hình CFS. Kết quả phân tích chi tiết được trình bày trong báo cáo.
Input format của CFS vẫn được giữ nguyên như MLQ nên thầy/cô có thể điều chỉnh file input để kiểm tra.

---

## So sánh ngắn giữa CFS và MLQ

| Tiêu chí               | CFS                                              | MLQ                                                    |
|------------------------|--------------------------------------------------|--------------------------------------------------------|
| Công bằng              | Cao, ít xảy ra starvation                        | Thấp hơn, dễ gây starvation                            |
| Context Switch         | Nhiều hơn do nhiều process->thời gian chạy ngắn  | Phụ thuộc vào quantum, thường ít hơn                   |
| Độ phù hợp             | Phù hợp với hệ thống tương tác                   | Phù hợp với hệ thống thời gian thực (real-time)        |
| Độ phức tạp hiện thực  | Khó hơn, nhất là cây đỏ đen                      | Dễ hiện thực hơn                                       |

Nhóm không so sánh **Turnaround Time (TAT)** giữa hai mô hình vì hệ điều hành hiện thực đơn giản, không tính chi phí Context Switch. Do đó, cả hai mô hình đều có thể đạt TAT tối thiểu bằng cách giảm `target_latency` hoặc `time_slot` để mỗi lần process chỉ được phép chạy 1 lệnh.

---

## Các cải tiến thêm ngoài yêu cầu

- Thêm phần in ra TAT và Waiting Time sau khi chương trình kết thúc.
- Mô hình MLQ giờ đây có thể nhận giá trị `prio` âm (tự động cộng thêm 20 nếu phát hiện có prio âm) để chạy chung test với CFS.
- Syscall `killall` đã được điều chỉnh để hoạt động trên cả mô hình CFS.
