import pandas as pd

# Đọc file CSV đã export từ Wireshark
df = pd.read_csv('test1.csv')

# Lọc các gói TCP có chứa dữ liệu và ACK
tcp_data = df[df['Protocol'] == 'TCP']

# Tách 2 chiều để phân tích
from_A_to_B = tcp_data[(tcp_data['Source'] == '192.168.246.232') & (tcp_data['Destination'] == '192.168.246.153')]
from_B_to_A = tcp_data[(tcp_data['Source'] == '192.168.246.153') & (tcp_data['Destination'] == '192.168.246.232')]

# Tạo list lưu kết quả delay
delays = []

# Lặp qua từng gói gửi từ A đến B
for index, row in from_A_to_B.iterrows():
    seq = None
    # Tìm chuỗi Seq=xxxx
    if 'Seq=' in row['Info']:
        seq = int(row['Info'].split('Seq=')[1].split()[0].split(',')[0])

    if seq:
        # Tìm gói ACK từ B đến A có Ack=seq + len (ở đây len=1234)
        ack_val = seq + 1234
        matching_ack = from_B_to_A[from_B_to_A['Info'].str.contains(f"Ack={ack_val}", na=False)]
        if not matching_ack.empty:
            ack_time = matching_ack.iloc[0]['Time']
            send_time = row['Time']
            delay = float(ack_time) - float(send_time)
            delays.append(delay)

# Xuất thống kê
if delays:
    print(f"Số mẫu delay: {len(delays)}")
    print(f"Delay trung bình: {sum(delays)/len(delays)*1000:.2f} ms")
    print(f"Delay lớn nhất: {max(delays)*1000:.2f} ms")
    print(f"Delay nhỏ nhất: {min(delays)*1000:.2f} ms")
    print(f"Số gói vượt 500ms: {sum(1 for d in delays if d > 0.5)}")
else:
    print("Không tìm thấy cặp Seq/Ack phù hợp.")
