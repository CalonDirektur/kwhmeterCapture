# Aplikasi Monitoring KWH Meter dengan IoT dan Telegram

## Deskripsi
Aplikasi ini dirancang untuk memantau konsumsi listrik melalui KWH meter menggunakan perangkat IoT. Data konsumsi listrik dikirimkan ke pengguna melalui bot Telegram, memungkinkan pemantauan yang efisien dan real-time.

## Fitur
- **Pemantauan Real-time**: Menggunakan sensor IoT untuk membaca angka KWH meter secara langsung.
- **Notifikasi Telegram**: Kirim notifikasi ke Telegram untuk pembaruan konsumsi listrik.
- **Analitik Konsumsi**: Menyediakan analisis penggunaan listrik secara historis.
- **Konfigurasi Mudah**: Memungkinkan pengguna untuk mengkonfigurasi dan mengelola pengaturan perangkat dan notifikasi.

## Teknologi yang Digunakan
- **Arduino/ESP32 Cam**: Sebagai perangkat utama untuk menangkap data dari KWH meter.
- **Telegram Bot API**: Mengirimkan notifikasi dan menerima perintah dari pengguna.
- **Platform Cloud**: (Opsional) Menyimpan data di cloud untuk pemantauan dan analisis lebih lanjut.

## Instalasi
1. **Persiapan Perangkat Keras**: 
   - Pasang sensor ke KWH meter (jika ada).
   - Hubungkan sensor ke perangkat IoT (Arduino).

2. **Pengaturan Perangkat Lunak**:
   - Clone repository ini:
     ```https://github.com/CalonDirektur/kwhmeterCapture.git```
  - Buka file kode Arduino (`.ino`) dengan Arduino IDE.
  - **Pengaturan Token Telegram**: Di dalam kode, pastikan untuk memasukkan token bot Telegram Anda yang didapat dari [BotFather](https://core.telegram.org/bots#botfather).

3. **Mengunggah Kode ke Arduino**:
   - Hubungkan Arduino ke komputer.
   - Pilih port yang benar di Arduino IDE.
   - Klik `Upload` untuk memprogram Arduino dengan kode.

## Cara Menggunakan
- Tambahkan bot Telegram Anda untuk memulai pemantauan.
- Kirim perintah `/start` untuk memulai interaksi dengan bot.
- Bot akan secara otomatis mengirim pembaruan konsumsi listrik sesuai interval yang ditentukan dalam kode Arduino.

## Kontribusi
Kontribusi sangat dihargai! Silakan fork repository ini dan kirimkan pull request dengan ide atau perbaikan Anda.

## Lisensi
Proyek ini dilisensikan di bawah [MIT License](LICENSE).

## Kontak
Untuk pertanyaan lebih lanjut, silakan hubungi [afdhal.syahrullah@gmail.com](mailto:afdhal.syahrullah@gmail.com).
