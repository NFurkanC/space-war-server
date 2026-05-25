# Space War Online

Bu proje, orijinal **Space War** oyununu çevrimiçi  bir yapıya taşırken istemci-sunucu arasındaki veri akışını optimize etmeyi ve modüler bir mimari kurgulayarak Spring Boot öğrenmeyi hedefleyen bir çalışmadır.

## Projenin Amacı
- **Ağ Verimliliği:** JSON yerine ikili (binary) format (`ByteBuffer`) kullanarak ağ üzerindeki veri yükünü minimize etmek.
- **Modüler Mimari:** Spring Boot'un sunduğu servis ve WebSocket yapılarını kullanarak ölçeklenebilir bir oyun motoru altyapısı kurmak.
- **Performans:** Sunucu tarafında "Spatial Partitioning" (Mekansal Bölümleme) kullanarak binlerce nesnenin olduğu haritalarda bile yüksek performans sağlamak.

## Çalışma Mantığı (Server-Authoritative)
Oyun, tüm oyun mantığının sunucuda döndüğü bir modelle çalışır:
- **Sunucu (Spring Boot):** Oyun döngüsünü (Game Loop) koşturur, çarpışma kontrollerini yapar ve `Map` üzerindeki grid yapısı sayesinde sadece oyuncunun çevresindeki veriyi iletir.
- **İstemci (JS):** WebSocket üzerinden gelen binary verileri `pako` kütüphanesiyle açar ve HTML5 Canvas kullanarak görselleştirir.

## Proje Klasör Yapısı ve Sorumluluklar

### 1. `src/main/java/com/hiddenoob/Math`
Oyunun geometrik ve fiziksel hesaplama çekirdeğidir.
- **Vektör & Çizgi Hesaplamaları:** Temel geometrik işlemler ve vektör matematiği.
- **PolygonBuilder:** Gemiler, asteroidler ve mermiler için poligon yapılarını programatik olarak inşa eder.

### 2. `src/main/java/com/hiddenoob/space_war_server/gameObjects`
Oyun dünyasındaki canlı varlıkların (Entity) yönetildiği katmandır.
- **Player:** Oyuncu verilerini ve `WebSocketSession` bağlantılarını barındırır.
- **Map (Spatial Partitioning):** Dünyayı hücrelere bölerek çarpışma kontrollerini $O(n^2)$ karmaşıklığından kurtararak optimize eder.

### 3. `src/main/java/com/hiddenoob/space_war_server/packets`
Veri transfer katmanıdır.
- **Binary Serialization:** Veriyi doğrudan `ByteBuffer` üzerinden ham formatta paketler.
- **UniformListPacket:** Aynı türdeki paketleri (örn: asteroid listesi) tek bir seferde, başlık bilgisini tekrarlamadan verimli şekilde gönderir.

### 4. `src/main/resources/static`
HTML5 ve JavaScript tabanlı istemci bileşenleri.
- **network.js:** Sunucuyla WebSocket iletişimini ve gelen paketlerin decompress/decode işlemlerini yönetir.
- **renderer.js & camera.js:** Oyun dünyasının çizimi ve yumuşatılmış kamera takibi.

---
*Bu proje geliştirme aşamasındadır ve Spring Boot öğrenme sürecinin bir parçasıdır.*

## SDL Derleme Süreci
`&& cd src\main\typescript\SDL`

`cmd.exe /c '"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"  && mkdir build && cd build && "C:\Program Files\CMake\bin\cmake.exe" -G "Ninja" .. && "C:\Program Files\CMake\bin\cmake.exe" --build .'` 

Bagımlılıklar otomatik olarak yüklenecektir. Lütfen klasör pathinizi kendi cihazınıza göre güncelleyiniz. SDL2d.dll dosyasını derlenen çalıştırılabilir uygulama ile aynı konumda bulundurmanız gerekebilir. 