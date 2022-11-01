# STM32L4-Bootloader (Bare-metal, Board Support Package, Register Level)

Gömülü sistem mühendisleri saha hataları kolayca handle etmek veya bir sensör
networküne bağlı olan 100 üzerindeki cihazın her birine tek tek yazılım yüklemek yerine
alternatif yöntemlere yönelmişlerdir. Bunun sonucunda yazılımcılar işlemci üzerinde
koşan kodun farklı haberleşme kabloları veya kablosuz alternatif methodlar sayesinde
sistem üzerindeki mikroişlemciyi programlayabilecek yöntemler geliştirmişlerdir.
Günümüze kadar işlemci üreticileri bu sorunu giderebilmek ve kullanıcılara daha iyi bir
geliştirme ortamı sunmak için In Application Programming(IAP) veya In Circuit
Programming gibi seçenekler sunmuştur. Bunlardan IAP versiyonu gömülü sistemin
firmware dosyasını dış haberleşme aygıtları(UART,SPI,I2C,WIFI,Ethernet) gibi
ortamlardan alabilmesini ve hedef gömülü sistemin yazılımının değiştirilmesini
amaçlamıştır. Bu sayede UART veya kablosuz ortam(Bluetooth) gibi ortamlardan alınan
datalar firmware olarak kullanılabilecekti. Bu sayede gömülü sistemin bağlı olduğu
platforma ait herhangi bir data yoluna girilerek hedef işlemciye ulaşıp firmware dosyası
yüklenebilir.
Gömülü sistemlerde birincil bootloader, işlemci üzerinde kullanıcı uygulamasını ayağa
kaldırmak için kullanılır. İşlemciye güç verildiğinde veya reset atıldığında öncelikle
birincil bootloader ayağa kalkar ve işlemciye yüklenen uygulama kodunu işleme aldıktan
sonra otomatik kendini sonlandırır.
Kısacası yapılacak işlem sayesinde kod güncellemesi uzaktan yapılarak sahada meydana
gelen problemlerin veya saha çalışanına ihtiyaç ortadan kaldırılmak amaçlanmıştır. Bu
durumda birincil önyükleyici sadece işlemcinin yapması gereken işlemlerini ilgili adreslere
iletecektir ve ikincil önyükleyici tarafımdan yazılarak istenilen uygulamalar arasında geçiş
yapılacaktır.

## 1- PLANLAMA
Planlamaya göre bootloader içerisinde UART haberleşme protokolü, 2 adet uygulama, kartın flashının 
yönetimi ve istenilen adreslere zıplayarak çalışmasını sağlayan jumper modülü yer alacaktır.
Kullanıcı ara yüzünün oluşturulması için TeraTerm isimli seri port haberleşme uygulaması UART 
üzerinden kontrol edilecektir. STM32’yi programlamak için önerilen STM32CubeIDE kurulacak ve
kodlama işlemleri gerçekleştirilecektir. Bu durumda haberleşmenin sekteye uğramaması için bir 
USB-TTL kablosu kullanılacaktır ve UART pinlerine
aşağıdaki şekildeki gibi bağlanacaktır.

![image](https://user-images.githubusercontent.com/70964563/199352872-580e9817-5aa5-4c54-b7d5-1db112132a0c.png)

## 2- YAN UYGULAMA GÖREVLERİ
### 2.1-Birinci uygulama APP-1: 
UART ile haberleşmesi ile terminal ekranında “Birinci
uygulamadan merhaba” mesajı gönderecektir. Bu işlem yapılırken board üzerinde
bulunan PC7 hattına bağlı olan yeşil led sürekli blink edecek ve birinci uygulamada
çalıştığı görülecektir.
### 2.2-İkinci uygulama APP-2: 
UART ile haberleşmesi ile terminal ekranında “İkinci
uygulamadan merhaba” mesajı gönderecektir. Bu işlem yapılırken board üzerinde
bulunan PB14 hattına bağlı olan kırmızı led sürekli blink edecek ve birinci
uygulamada çalıştığı görülecektir.

Bu uygulamalar board support package olarak adlandırılan BSP, register level ve Bare-Metal olarak 
gerçekleştirildiği için normalde bir satırda halledilecek olan bir ledin blink etmesi için ilk 
olarak STM32’nin blok diyagramından ledlerin bağlı olduğu RCC hattı açılmıştır. Daha sonra GPIO 
hattının ODR registerını çıkış olarak ayarlanıp ledlerin bağlı olduğu 7 ve 14. pinlerindeki ilk 
durumları high, arada bir bekleme süresi ile daha sonra low olarak atanmıştır. Ledlerin high veya 
low olarak bekleme süreleri STM32’nin çalışma prensibine göre 4MHz üzerinden hesaplanarak 
1 saniye high, 1saniye low olarak ayarlanıp ledlerin blink etmesi sağlanmıştır. Bu aşamada bu 
iki uygulama da flashın base adresinde çalıştırılmıştır ve ileride modifiye edileceklerdir.

## 3-UART HABERLEŞME PROTOKOLÜ
UART (Universal Asynchronous Receiver Transmitter), bilgisayar ve mikrokontroller veya
mikrokontroller ve çevre birimler arasında haberleşmeyi sağlayan haberleşme protokolüdür.
Asenkron olarak çalıştığı için herhangi bir “clock” ihtiyacı duymaz. USART (Universal
Synchronous Asynchronous Receiver Transmitter) ise hem senkron hem de asenkron olarak
çalışabilir.
USART, UART’a göre daha gelişmiş bir protokoldür. Haberleşme mantıklı aynı şekilde
çalışır ancak USART aynı zamanda senkron haberleşmeleri de gerçekleştirebilir. UART
haberleşmesini gerçekleştirirken ilk olarak baudrate (veri taşıma hızı) ayarlanması gerekir.

Veri taşıma hızı çok çeşitli aralıklarda olabilir ancak piyasada yaygın olarak kullanılan baudrate’ler 4800,
9600, 57600, 115200’dür. Bu projede veri hızı 115200 olarak belirlenmiştir. Veri iletimi için aşağıdaki ve
aşağıdaki görseldeki gibi bir yapı kullanılır.
Haberleşme işlemimiz bir başlangıç bitinden sonra data bitleri, ardından parity biti ve son olarak da bitiş
biti gönderilerek sonlandırılır. Bu işlem sırasında data uzunluğu ve parity biti opsiyonel
olarak değişkenlik gösterebilir.

![image](https://user-images.githubusercontent.com/70964563/199353333-c535f60f-a183-402d-b985-160fa27520b3.png)
![image](https://user-images.githubusercontent.com/70964563/199353500-a7db5f83-f1d0-486f-baba-67f1fcbb64d3.png)

Bu proje kapsamında UART protokolü terminal ekranında bir kullanıcı arayüzü oluşturacağı
için sürekli veri alışverişi yapacaktır. Bu nedenle interrupt modda kullanılmıştır. İnterrupt
mod aktifken yapılan ayarlama sayesinde terminalden mesaj alınması durumunda interrupt
fonksiyonu aktif olacaktır. Bu işlemlerin yapılabilmesi için yine STM32’nin blok
diyagramına ve datasheetine bakılarak USART3 hattının kullanılması gerektiği görülmüştür.
USART3 hattının pinleri STM32’nin şematiğinden bakılarak PC5 ve PC4 olarak
görülmüştür. Bu duruma uygun olarak USB-TTL bağlantısı yapılmıştır. UART hattının aktif
olması için RCC üzerinden C portu aktif edilmiştir. C portunun aktif olması ile GPIO hattında
C portu alternate function yani analog output olarak ayarlanmış ve programlama
kılavuzundan bakılarak AFR registerlarına gerekli değerler atanmıştır. Bu sayede artık
UART protokolü için gerekli ön hazırlık tamamlanmış ve UART protokolü aktif hale
getirilmek için baudrate ve control registerlarına istenen değerler atanmıştır. Aynı
fonksiyonun içerisinde UART aktif edilmiştir ve son olarak interrupt mod çalıştırılmıştır.
Bu işlemler sayesinde terminal üzerinde bir kullanıcı arayüzü oluşturmak için zemin
hazırlanmıştır.
UART protokolü sayesinde oluşturulacak olan arayüz sürekli olarak karşılıklı iletişim
kurmalıdır. Bu işlemler için sürekli olarak terminalden kullanıcı girişi dinlenmelidir.

İlk adımda eğer 10sn herhangi bir tuşa basılmazsa default olarak ikincil önyükleyici devreye 
girecek yapı hazırlandı. Bu yazı ekrana mesaj olarak gönderilip kullanıcıdan herhangi bir 
giriş var mı diye kontrol etmektedir. 

![image](https://user-images.githubusercontent.com/70964563/199353901-cb9f7c46-5a39-4934-9ffb-528f6447f815.png)

İkinci adımda eğer herhangi bir giriş olmazsa default başlangıcı aktif ettiğini gösteren mesaj gönderilmiştir.

![image](https://user-images.githubusercontent.com/70964563/199353993-03af87b2-4cb7-4004-ba68-26d130711b3a.png)

Üçüncü adımda eğer kullanıcı 10 saniye içerisinde herhangi bir giriş yaptıysa önüne bir menü
çıkarması için tasarlanmıştır. Böylece kullanıcı menüden bir durum seçebilmektedir.

![image](https://user-images.githubusercontent.com/70964563/199354048-83635fc2-8c53-4b53-a2e8-c91c3c6307e4.png)

Menüden de görüldüğü gibi eğer kullanıcı klavyeden 1 tuşuna basarsa yüklenen 1. Uygulama
devreye girecektir. Kullanıcı 2 tuşuna basarsa yüklenen 2. Uygulama devreye girecektir. 
Eğer 1. uygulamayı silmek isterse 3’e, 2. uygulamayı silmek isterse 4’e basarken silinen
uygulamaların yerine yeni bir uygulama yükleyecekse 5’e basmalıdır. Klavyeden 5’e basan
kullanıcı hangi uygulamanın yerine yeni uygulamayı yükleyeceğini seçmelidir. Bu
girişlerden sonra istenilen uygulamanın binary(.bin) dosyası yüklenerek uygulamanın
güncellenmesi sağlanacaktır. Eğer kullanıcı 6’ya basarsa 1. Uygulama silinip yeni uygulama
yüklemelidir. Eğer kullanıcı 7’ye basarsa 2. Uygulama silinip yeni uygulama yüklemelidir.

![image](https://user-images.githubusercontent.com/70964563/199354118-4cc8f44d-4109-4d63-9d45-15f235981414.png)

## 4-FLASH YAPILANDIRMASI
![image](https://user-images.githubusercontent.com/70964563/199354217-321021bb-ba2d-43e8-8b3f-368f87f47124.png)
![image](https://user-images.githubusercontent.com/70964563/199354700-022f1145-50aa-47da-83e1-53d1b9033f2c.png)

Yukarıdaki görsellerden de görüldüğü gibi flash görselinin elde edilmesi için vectör adresi
işlemlerin yapılması gerekmektedir. CMSIS kütüphanesindeki bu değişiklikler linker
içerisinde işlenerek STM32CubeIDE içerisinde algılanarak STM32 içerisinde bulunan flash
yapısını soldaki şekle getirmektedir. Bu durumda dikkat edilmesi gereken şeylerden birisi iki
uygulama arasında boş bir alan bırakmaktır. Aksi halde bazı aksaklıklar meydana
gelebilmektedir. STM32L4 modelinde 1MB flash boyutu olduğu için en uygun bölme şekli
gözetilmiştir. Bu sayede önyükleyicinin zıplama işlemi için gerekli zemin
hazırlanmıştır.

Yukarıdaki işlemleri kendi kod bloğumuza dahil etmek için aşağıdaki gibi define işlemi yapmak
kod yazarken her satırda kolaylık sağlayacaktır. 

![image](https://user-images.githubusercontent.com/70964563/199355398-1fcd5c79-5440-4e97-bed1-e3e5fc477b46.png)

Bu işlemlerden sonra istenen durumda birinci uygulamaya zıplaması için MSP değeri uygulamaların 
bulunduğu adreslere atanmalıdır. Burada bilinmesi gereken bir formül vardır 

ResetHandlerAddress = MSPValue + 4

MSP değerinin +4 fazlası ise ResetHandler fonksiyonu olacak ve kullanıcı menüden 1. Uygulamaya gitmek 
isterse ResetHandler değeri 0x08040004 değeri alarak çipe reset atar ve 1. uygulamaya  
zıplayarak 1. Uygulamayı çalıştıracaktır. Eğer kullanıcı menüden 2. uygulamaya gitmek isterse 
ResetHandler değeri 0x08080004 değeri alarak çipe reset atar ve 2. uygulamaya zıplayarak 2. Uygulamayı 
çalıştıracaktır. Eğer kullanıcı hatalı bir giriş yaparsa ResetHandler değeri FLASH_BOOT değerinin +4 
fazlasını alarak çipe reset atıp tekrardan giriş yapmasını isteyecektir.

### 4.1- Flash Kilitleme ve Kilit Açma

![image](https://user-images.githubusercontent.com/70964563/199355687-b81c6dfd-c5dc-4568-a0bc-fbdaf4ecff91.png)

Flash kilitleri STMicroelectronics firması tarafından çipi koruma amaçlı eklenen bir yapıdır.
Flash default olarak kilitli gelmektedir ve referans kılavuzunda verilen şifreler ile flash kilidi
açılabilmektedir. Flash ile ilgili bir işlem yapmadan önce flashın kilidinin açılması
gerekmektedir. Yapılacak olan işlem yapılır yapılmaz flash tekrar kilitlenir ve bu sayede
kılavuzun önerdiği şekilde flash işlemleri gerçekleştirilmiş olacaktır.
Flash işlemleri aşağıdaki sıra ile yapılmalıdır.

1. Flash Kilidini Açma:

![image](https://user-images.githubusercontent.com/70964563/199355990-dc208ab4-d684-44a9-8816-ce41b4c8e55a.png)
Verilen 2 anahtar yukarıdaki şekildeki registera yazılarak flashın kilidi açılır.

2. Yapılmak İstenen İşlemlerin Yapılması:

İlerleyen aşamalardaki bütün işlemler bu noktada yapılacaktır.

3. Flashın Kilidini Kapatma:(STM32L4ZGT6 için CR registerında 31. Sırada bulunan LOCK biti 1 yapıldığında flash kilitlenir)

![image](https://user-images.githubusercontent.com/70964563/199356134-1b1e1bf2-2e0e-4e7f-9586-8a4a97aaeb54.png)




### 4.2- Flashtan Veri Silme
![image](https://user-images.githubusercontent.com/70964563/199356222-f3e28657-2d04-4600-80cd-fe822657c0ed.png)

Flash silme işlemleri yukarıdaki şekilde gösterilen flash yapısının temizlenmesini sağlayacaktır.
Fakat flash üzerinde herhangi bir işlem yapılabilmesi için flash içerisindeki bankaların ve
sayfaların yapısı bilinmelidir. STM32L4’ün flash yapısında sector bulunmamaktadır. Bunun
yerine her birisi 2Kb olan sayfalar yer almaktadır. Bu durumda sayfalar bankaları ve bankalar
flashı oluşturur. Bu durumda yapılacak silme işlemi için birden çok silme işleminin aynı anda
yapılması gerekmektedir.
Bu proje kapsamında 2 adet uygulama olacağı için bu uygulamaların gerektiğinde silinmesi
işlemi yapılmalıdır. 

64 K Hesaplama işlemi: 1 KiloByte = 1024 Byte, 64*1024 = 65536 for decimal, 65536 = 10000 for hex

• APP-1’i silmek için kapladığı alan 64Kb olarak işaretlendiyse silinmesi gereken sayfa
sayısı 32’dir. Bu durumda APP-1’i flashtan silme işlemi aşağıdaki sıra ile
yapılmalıdır:
1. Flash kilidinin açılması
2. Silinecek bölümün(0x08040000 – 0x08050000) ait olduğu banka işaretlenir.
3. Silinecek bölümün(0x08040000 – 0x08050000) ait olduğu sayfalar işaretlenir.
4. Silinecek sayfalar için bir döngü oluşturulur ve 32 adet sayfa silinir.
5. Flash kilitlenir.

• APP-2’i silmek için kapladığı alan 64Kb olarak işaretlendiyse silinmesi gereken sayfa
sayısı 32’dir. Bu durumda APP-2’i flashtan silme işlemi aşağıdaki sıra ile
yapılmalıdır:
1. Flash kilidinin açılması
2. Silinecek bölümün(0x08080000 – 0x08090000) ait olduğu banka işaretlenir.
3. Silinecek bölümün(0x08080000 – 0x08090000) ait olduğu sayfalar işaretlenir.
4. Silinecek sayfalar için bir döngü oluşturulur ve 32 adet sayfa silinir.
5. Flash kilitlenir.

Flashta herhangi bir işlem yapılmadığında STM32CubeİDE üzerinde yüklenen bir kod
flashın base adresine(0x08000000) yazılır. Bu durumda ön yükleyici bir elektrik kesintisinde
veya usb kablosunun çıkarılıp takıldığı zaman base adresteki işlemi çalıştırır. Fakat bu adrese
flashlama işlemi STM32CubeİDE tarafından otomatik olarak yapılır.

0x08000000 adresine yazılan bir kod bloğu otomatik olarak Bank 1 içerisinde yer almaktadır.
Bankanın silinmesi durumunda ön yükleyici kodunun kaybolması ve silinmesi söz konusu
olacağı için ön yükleyici korumaya alınmalıdır. Fakat 2. Uygulama silinirken 2.bank silinebilir.
Bu durumda yine 2 farklı işlem yapmamız gerekmektedir.

![image](https://user-images.githubusercontent.com/70964563/199357151-ba3a668a-b5a3-4c94-a270-063f7aca2c70.png)

Yukarıdaki şekilde de görüldüğü gibi APP-1 ve APP-2 farklı bankalarda bulunmaktadır.

Bank 1’i silmek için önyükleyici için ayrılan 256Kb alan pas geçilir. Başlangıç adresi 1.
Uygulamanın adresi olacaktır. 0x08040000’ten başlayarak 0x08080000 adresine kadar silme
işleminin yapılması gerekmektedir ve bu işlemler yine aşağıdaki sıra ile yapılmalıdır.

• BANK-1’i silmek için kapladığı alan 256Kb olarak işaretlendiyse silinmesi gereken
sayfa sayısı 128’dir. Bu durumda BANK-1’i flashtan silme işlemi aşağıdaki sıra ile
yapılmalıdır:
1. Flash kilidinin açılması
2. Silinecek bölümün(0x08040000 – 0x08080000) ait olduğu banka işaretlenir.
3. Silinecek bölümün(0x08040000 – 0x08080000) ait olduğu sayfalar işaretlenir.
4. Silinecek sayfalar için bir döngü oluşturulur ve 128 adet sayfa silinir.
5. Flash kilitlenir.

Bu işlemlerin haricinde BANK-2 silinmek isterse yapılması gereken işlemler içerisinde ön
yükleyici olmadığı için diğerlerine göre daha kolaydır ve yine aşağıdaki sıra ile yapılmalıdır:

• BANK-2’i silmek için kapladığı alan 512Kb olarak işaretlendiyse silinmesi gereken
sayfa sayısı 256’dir. Bu durumda BANK-2’i flashtan silme işlemi aşağıdaki sıra ile
yapılmalıdır:
1. Flash kilidinin açılması
2. Bank-2’nin silinmesi için FLASH_CR registerına MER2 biti 1 yapılır.(Şekil-21)
3. Flash kilitlenir

### 4.3-Flasha Veri Yazma
Flasha yazma işlemi flashı silme veya flashta herhangi bir işlem yapmaktan çok daha
komplex bir işlemdir. Kartın herhangi bir zarar görmemesi ve ön yükleyiciye müdahale
edilmemesi için çok dikkat edilerek davranılmalıdır.
Flasha yazma işlemi için oluşturulması gereken “enum” ve “struct” kalıplarının tanıtılması
gerekmektedir. Bunlar sırası ile; Flash_LockTypedef, Flash_CacheTypedef,
Flash_ProcessTypedef, Flash_StatusTypedef kalıpları olacaktır.

![image](https://user-images.githubusercontent.com/70964563/199357411-c48c034a-e6a2-4468-a44f-99b9521a13af.png)

![image](https://user-images.githubusercontent.com/70964563/199357466-cf2914e9-007b-4e33-be7a-55244e02acb1.png)

Burada yapılan tanımlamalar sayesinde de ACR registerına kolayca atama işlemleri
gerçekleştirilebilecektir. Bu durumda bir flasha yazma rutini için fonksiyon yazıldığında
istenilen adrese sadece istenilen 32bitlik bir verinin yazıldığı görülmüştür. Fakat bizim bu
adrese yazacağımız uygulamamız bu yazma işlemlerini hem kaydırarak hem de bu
büyüklükteki veriyi binlerce yazmamız gerektiği için geliştirilmelidir.

STM32L4 serisinin adrese yazma işlemi “DoubleWord” olarak gerçekleşmetedir. Bu
durumda flasha bir yazma işlemi yapıldığı durumda aşağıdaki sıra takip edilmelidir.
1. Flash kilidi açılır.
2. 64bitlik veri yazılır.
3. Flash kilidi kapatılır.
DoubleWord özelliğinden dolayı 64 bit yazma işlemi yapıldıktan sonra her sefer flash
kilitlenir. 64bit = 8Byte ise 80Byte bir yazma işlemi için yukarıdaki işlemlerin 10 kere
tekrarlanması gerekmektedir. Bu işlem sırasında her sefer 8 byte öteleme ile yeni yazma
yerinin adresi belirtilir. Aksi takdirde MCU hep aynı adrese yazmaya çalışacak ve flash yapısı
zarar görecektir.

![image](https://user-images.githubusercontent.com/70964563/199357590-a17d2413-58ab-4d1c-b7cc-86c7dd0d604e.png)

Yukarıdaki şekilde Address parametresi APP-1 için 0x08040000 değeri alırken, APP-2 için
0x08080000 değeri alacaktır. Data parametresi ise deneme sırasında manuel olarak
denenebilir fakat ilerleyen zamanlarda UART üzerinden gelen veri ile eşleştirilecek ve gelen
data belirtilen adrese yazılacaktır. DoubleWord yazma işlemi sırasında ilk 32 bitlik data
istenilen adrese yazılır ve diğer 32 bitlik data 32 bit kaydırılıp yazılmalıdır. Bundan önce tabi
ki adrese yazma işlemini belirtecek olan CR registerındaki PG biti aktif edilmelidir.
Bu fonksiyonların kendi içerisinde birbirini tekrar etmesi için recursive bir fonksiyon
tanımlanmalıdır ve kendi kendini çağırarak her iki işlemi de gerçekleştirecektir.

Bu işlemler sırasında SR registerındaki hata bayrakları kontrol edilmeli hata bayraklarının
low olduğu durumda EOP(en of operation) bayrağının set olduğu durumda yazma işleminin
tamamlandığı görülmelidir.

NOT: Eğer herhangi bir ERR bayrağı aktifse flasha yazma işlemi durdurulmalı ve flash
kilitlenerek korunup işlem bitirilmelidir. Aksi halde mikroişlemciniz zarar görebilir ve 
geri dönülemez bir yola girebilirsiniz.

Bu adımda da DoubleWord ve Recursive fonksiyonun iç içe yazılması yapılmıştır.
Adrese değer yazarken aynı zamandan gelen data shift edilecek ve 32bitlik bir işlemciye 64
bit şeklinde yazma işlemi yapılacaktır. Recursive fonksiyon her sefer kendi içerisinde 64 bit
yazarken FSM(Finite State Machine) mantığı devreye girecek ve gelen data ilgili adrese
yazarken PC = PC + 4 hesaplaması yapılarak bir sonraki yazma adresini hazırlamalıdır. Aksi
takdirde sistem çalışmayacak ve yazılan veri okunamaz olacaktır.

Yazılan recursive fonksiyonu ve FSM mantığı aşağıdaki akış diyagramında gösterilmiştir.
![image](https://user-images.githubusercontent.com/70964563/199357974-a5d1b94a-28d8-434d-b478-b41c1bbf03b5.png)


## 5- RAM YAPILANDIRMASI
UART protokolü FIFO(first giriş ve first çıkış) mantığına göre çalıştığı için uygulamaların
yüklenmesi sırasında bu işlemin binlerce hatta on binlerce kez tekrarlanması gerekmektedir.
APP-1 ve APP-2 uygulamaları yaklaşık olarak 5Kb büyüklüğe sahipken UART verisi 8bit
yani 1 byte olarak çalıştığı için UART üzerinden gönderilen uygulama dosyaları RAM
üzerine yazılmalı ve RAM üzerinden tekrar FLASH’ta istenilen adrese yazılmalıdır. Burada
yazılan kodların her zaman kullanılabilir kılınması için FLASH üzerinde uygulamalar için
ayrılan bölümün büyüklükleri kadar bir yapı oluşturulmalıdır.

• APP-1 ve APP-2 büyüklükleri: 64Kb = 64*1024*8=524288 bit.

Bu durumda oluşturacağımız yapı 524288 bitlik bir değişkeni saklayabilmelidir. Bu yapı
oluşturulup UART üzerinden gerekli atamalar yapılmıştır. APP-1 dosyasının binary karşılığı
olan dosya UART üzerinden gönderilmesi için Seçim ekranından ilk olarak 5 tuşuna basılıp,
ikinci adımda hangi dosya yerine yeni uygulama yükleneceği seçilmelidir. Daha sonra
TeraTerm ekranından dosya gönderilirken ayarlar kısmından binary olarak işaretlenip,
uygulamanın binary eşdeğeri UART üzerinden gönderilir. Bu işlemler yapıldıktan sonra
RAM kontrol edilerek istenilen değerlerin alınıp alınmadığı kontrol edilir. RAM üzerine
yazılan dosyalar bir sonraki adımda ise FLASH’a aktarılacak ve proje tamamlanacaktır.

![image](https://user-images.githubusercontent.com/70964563/199358687-7ba29f50-44e2-4e4f-a15e-f9ba6b6e04c5.png)

Artık UART üzerinden alınan verinin RAM üzerinden FLASH’a yazılma işlemi yapılmıştır. 
“Hexedit” programı sayesinde binary dosyalar hexeadecimal olarak görüntülenebilmektedir. 
“ST-Link Utility” programı sayesinde de FLASH içeriği görüntülenebilmektedir. 
Yapılan işlemlerin testleri bu iki programdaki ASCII karakterlerin karşılaştırılması ile yapılacaktır.
RAM üzerindeki veriyi FLASH içerisine aktarabilmek için 4 sütun 128bitten oluşan FLASH
içeriğine göre matematiksel olarak bölerek ve bir döngü içerisinde sürekli öteleme işlemi
yapılıp, 4 sütun tamamlandığında ise bir alt satıra geçmesi için yeni bir fonksiyon eklenmiştir.
Testler sırasında Little-Endian ve Big-Endian kavramları önem kazanmıştır. Little-Endian
küçük bite küçük değeri atama yaparken, Big-Endian küçük bite büyük değeri atama işlemi
yapacaktır. Örneğin: Big-Endian bir bilgisayarda, 4F52 onaltılık sayı için gereken iki bayt,
depolamada 4F52 olarak depolanır. Little-Endian sisteminde, 524F olarak depolanacaktır.
Şekilde gösterilen iki programda farklı Endian sistem kullandığı için hexadecimal sayıların
yerleri farklılık gösterse bile ASCII çıktıları karşılaştırıldığında aynı verileri barındırdıkları
gözlemlenmiştir.

• 0x08040000 adresine APP-1.bin dosyası yüklendiğinde:

![image](https://user-images.githubusercontent.com/70964563/199358986-43e43bd8-c803-465a-b08d-bd98eeca82f1.png)

• 0x08080000 adresine APP-2.bin dosyası yüklendiğinde:

![image](https://user-images.githubusercontent.com/70964563/199359084-9adc768c-0e51-4ad4-9d7c-4a58373be91b.png)

Yukarıdaki işlemler sayesinde aktarılan ve flasha yazılan iki uygulama da kontrol edilmiştir.
Bu projede önemli olan aktarım sırasında kayıpların meydana gelip gelmediği ve kontrollü aktarım
için bir CRC algoritmasının geliştirilmesi gereklidir. İlerleyen günlerde güncelleme ile
bu eksikliği kapatacağım.


## 6 - KULLANIM KLAVUZU

Terminal ekranı yönlendirmesi ve çalışma prensibi aşağıdaki gibidir.

### 6.1- Default Screen
![image](https://user-images.githubusercontent.com/70964563/184142733-299da9a9-5ba1-43bc-af73-1250fce1ec7a.png)

### 6.2- Eğer herhangi bir tuşa basılmazsa aşağıdaki ekran görülür.
![image](https://user-images.githubusercontent.com/70964563/184143040-c20154c8-c975-4687-bd5e-85eb32eb3319.png)

### 6.3- Eğer herhangi bir tuşa basılırsa aşağıdaki ekran görülür.
![image](https://user-images.githubusercontent.com/70964563/184143150-49a39bf6-e367-4506-a98f-e2b28a753949.png)

### 6.4- Eğer 0'ı seçerseniz default olarak yüklenen pc7 ve pb14 ledlerinin blink ettiğini ve şağıdaki ekran görülür.
![image](https://user-images.githubusercontent.com/70964563/184143444-bee2610b-d94e-4e5f-a8d8-79a3b74e1d18.png)

### 6.5- Eğer 1'i seçerseniz pc7 ledinin blink ettiğini ve aşağıdaki ekran görülür.
![image](https://user-images.githubusercontent.com/70964563/184144426-eb21772c-69a1-47a2-9f20-a4960ee7de2a.png)

### 6.6- Eğer 2'yi seçerseniz pb14 ledinin blink ettiğini ve aşağıdaki ekran görülür.
![image](https://user-images.githubusercontent.com/70964563/184144623-4c8e1dc4-4392-4d07-a897-4f0148ed56f9.png)

### 6.7- Eğer 3'ü seçerseniz APP-1 silinir ve aşağıdaki ekran görülür.
![image](https://user-images.githubusercontent.com/70964563/184144785-4ef76eab-2337-40c6-99bd-4462125450db.png)

### 6.8- Eğer 4'ü seçerseniz APP-2 silinir ve aşağıdaki ekran görülür.
![image](https://user-images.githubusercontent.com/70964563/184144955-66f484f9-416e-479e-be55-d900a8c8e93f.png)

### 6.9- Eğer 5'i seçerseniz alt menülerden seçim yapmanızı gerektiren ekran görülür.
![image](https://user-images.githubusercontent.com/70964563/184145131-8217bbe9-ee52-4818-8fab-6fbdc98ed050.png)

## 6.10- Eğer 6'yı seçerseniz File->SendFile->APP1.bin dosyası seçilerek Tera-Term üzerinden gönderim sağlanır.
![image](https://user-images.githubusercontent.com/70964563/184145542-a34f5b57-f12c-4869-b01e-706329fcc7e3.png)
APP yükleme işlemini kontrol etmek isteseniz STM üzerindeki reset butonu ile terminal ekranını başa sarıp 6.5'teki ekranı görebilirsiniz.

## 6.11- Eğer 7'yi seçerseniz File->SendFile->APP2.bin dosyası seçilerek Tera-Term üzerinden gönderim sağlanır.
![image](https://user-images.githubusercontent.com/70964563/184145846-b87c6b97-35f7-4f6b-979b-9f9c9c03d7a5.png)
APP yükleme işlemini kontrol etmek isteseniz STM üzerindeki reset butonu ile terminal ekranını başa sarıp 6.6'teki ekranı görebilirsiniz.
