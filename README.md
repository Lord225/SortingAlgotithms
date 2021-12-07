# Algorytmy Sortujące z Benchmarkami i Testami
Testowane na `i7 7700k@4.5ghz`
Wszystkie benchmarki to sortowanie dodatnich liczb całkowitych z zakresu od 0 do 10'000. 

<img src="Benchmarks%20Output/Figure_1.png"/>
<p float="left">
<img src="Benchmarks%20Output/Figure_12.png", width="49%"/>
<img src="Benchmarks%20Output/Figure_13.png", width="49%"/> :
</p>
<img src="Benchmarks%20Output/Figure_11.png"/>
Predykcje są oparte na parametryzowanych funkcjach:

```
y = xlog(x)
```

i 

```
y = x*x + x
```

W zależności od oczekiwanej złożoności obliczeniowej. 
Do dopasowania krzywej do danych użyto funkcji `curve_fit` z scikit-learn 

<img src="Benchmarks%20Output/Figure_3.png"/>
<img src="Benchmarks%20Output/Figure_31.png"/>
<img src="Benchmarks%20Output/Figure_32.png"/>
