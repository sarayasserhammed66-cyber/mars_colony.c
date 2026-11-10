# Mars Kolonisi Yasam Destek Simulatoru

C diliyle gelistirilmis, terminal tabanli bir hayatta kalma oyunudur. Oyuncu, Mars'taki bir arastirma istasyonunda tek basina kalan muhendisi yonetir. Amac, kurtarma ekibi 15. gunde gelene kadar saglik ve oksijen degerlerini sifirin ustunde tutmaktir.

## Ozellikler

- Farkli baslangic kaynaklari ve firtina sureleri olan uc zorluk seviyesi.
- `switch-case` ile calisan, buyuk/kucuk harfe duyarsiz komut menusu.
- Birden fazla rastgele sonucu olan kesif komutu.
- Her 2 birim suyun 3 birim oksijene donustugu oksijen uretimi.
- Hasar seviyesine gore modul tamiri.
- Dinlenme, durum raporu, kum firtinasi simulasyonu ve hava kilidi sifre denemesi.
- Gun ve tur takibi, kritik uyarilar, kazanma/kaybetme kosullari ve puanlama.

## Gereksinimler

GCC, Clang veya Visual Studio ile gelen Microsoft C derleyicisi gibi bir C derleyicisi gerekir.

## Derleme ve calistirma

GCC ile proje klasorunde bir terminal acip su komutu calistirin:

```sh
gcc -std=c11 -Wall -Wextra -pedantic -o mars_colony src/mars_colony.c
```

Windows'ta `mars_colony.exe`, macOS veya Linux'ta `./mars_colony` dosyasini calistirin.

## Komutlar

| Tus | Islev |
| --- | --- |
| `G` | Su veya yedek parca aramak icin kesif yapar; tehlikeyle karsilasabilir |
| `O` | Suyu oksijene donusturur |
| `T` | Yedek parca kullanarak modul hasarini onarir |
| `D` | Mevcut durumu gosterir |
| `U` | Dinlenerek enerji ve uygun kosullarda saglik kazandirir |
| `K` | Kum firtinasi simulasyonunu baslatir |
| `H` | Hava kilidi sifresini girmeyi dener |
| `Q` | Oyundan cikar |

Komutlar buyuk/kucuk harfe duyarsizdir. Durum komutu ve tanimsiz komutlar zamani ilerletmez. Her uc turde yeni bir gun baslar ve oksijen 8 azalir. Kum firtinasindaki her simulasyon turu oyun turu sayacina eklenir.

## Puanlama

Son puan su formulle hesaplanir:

`(gun x 100) + saglik + oksijen - modul hasari`

## 
