# GAME: Just Shapes and Shoot

## Thông tin sinh viên
- Họ và tên: Triệu Tiến Dũng
- MSSV: 24021436

## Giới thiệu
Game bắn súng 2D với người chơi điều khiển nhân vật di chuyển và bắn để tiêu diệt các kẻ địch hình tam giác và ngũ giác, đồng thời tránh các tia sáng tấn công từ cạnh màn hình.

## Ngưỡng điểm đề xuất: 7.5 - 8.5

## Các tính năng đã thực hiện

### Engine game và cơ chế cơ bản
- Sử dụng thư viện SDL2 và SDL2_image để xây dựng game 2D
- Hệ thống quản lý đối tượng game linh hoạt với đa hình (GameObject là lớp cơ sở)
- Quản lý trạng thái game (chạy, tạm dừng, kết thúc)
- Thiết kế kiến trúc hướng đối tượng với các lớp quản lý riêng biệt

### Đồ họa và hiển thị
- Hiển thị đồ họa 2D với texture cho người chơi và các đối tượng
- Hiệu ứng nhấp nháy khi nhân vật bị va chạm
- Hiệu ứng hoạt hình khi nhân vật chết
- Giao diện người dùng đơn giản (điểm số, thông báo tạm dừng)

### Cơ chế chơi
- Điều khiển nhân vật bằng bàn phím (WASD hoặc phím mũi tên)
- Hệ thống bắn đạn với hướng theo chuột, mở rộng cửa sổ theo hướng đạn bắn
- Ba loại kẻ địch khác nhau:
  - Tam giác: Di chuyển về phía người chơi
  - Ngũ giác: Có lượng máu cao hơn và di chuyển phức tạp hơn
  - Tia sáng: Xuất hiện từ cạnh màn hình với cảnh báo trước khi tấn công

### Vật lý và va chạm
- Hệ thống va chạm giữa các đối tượng game
- Cơ chế phản lực khi người chơi bị va chạm (knockback)
- Phát hiện va chạm chính xác giữa các hình dạng khác nhau

### Quản lý game
- Tạm dừng/tiếp tục game với phím ESC
- Khởi động lại game với phím R
- Thoát nhanh với phím P
- Hệ thống tính điểm khi tiêu diệt kẻ địch

## Các thuật toán đã cài đặt
- Thuật toán phát hiện va chạm giữa các hình dạng khác nhau
- Thuật toán sinh kẻ địch ngẫu nhiên từ các cạnh màn hình
- Thuật toán di chuyển của kẻ địch (theo dõi người chơi)
- Cơ chế phản lực (knockback) cho người chơi khi va chạm
- Hiệu ứng hoạt ảnh cho các sự kiện game

## Điểm nổi bật
- Thiết kế mã nguồn có tính mở rộng cao
- Sử dụng các kỹ thuật lập trình C++ hiện đại (smart pointers, move semantics)
- Tối ưu hóa hiệu suất với việc quản lý bộ nhớ cẩn thận
- Hệ thống điều khiển đáp ứng nhanh với cảm giác chơi mượt mà

## Nguồn tham khảo
- [Tài liệu SDL2 chính thức](https://wiki.libsdl.org/SDL2)
- [Ý tưởng cho lớp Vector2D](https://github.com/khris190/ReEngine/blob/7106e865e1985cdcd8b8359aa07fa3970c9a0f48/ReEngine/ReEngine/Re/Common/Math/Vector2D.h)

## Sử dụng AI
- Đã sử dụng AI vào những công việc sau:
  - Học về những thuật toán, cấu trúc được sử dụng trong project (SAT, cấu trúc polymorphism)
  - Viết comment, đề xuất tên biến sao cho thích hợp và dễ hiểu
  - Sắp xếp lại tổ chức file (ví dụ như file header player.h)
  - Autocomplete những phần code lặp lại (ví dụ như phần code switch direction trong beam.cpp)
  - Viết phần readme.md
