# cpp-simple-vector
Упрощённый аналог шаблонного класса vector из стандартной библиотеки.
Данные храняться в памятиб динамически выделяемой в куче. Память выделяется неинициализированная, инициализация происходит при фактическом добавлении элементов в вектор.
При добавлении новых элементов, если выделенной памяти недостаточно - выделяется новый участок памяти размером в два раза больше предыдущего, в который перемещаются
(либо копируются) данные из старого участка памяти, после чего старый участок удаляется.
## Работа с классом SimpleVector
#### Создание вектора:
Конструктор по-умолчанию, создающий пустой вектор.
```CPP
Vector<int> vec;
std::cout << vec.Capacity() << " " << vec.Size();
```
Конструктор, создающий вектор заданного размера (инициализирован значениями по умолчанию).
```CPP
Vector<int> vec(10);
std::cout << vec.Capacity() << " " << vec.Size() << std::endl;
```
Копирующий конструктор и перемещающий конструктор.
```CPP
Vector<int> vec1(5);
Vector<int> vec2(vec1);
Vector<int> vec3(std::move(vec1));
```
Операторы копирующего присваивания и перемещающего присваивания.
```CPP
Vector<int> vec1(5);
Vector<int> vec2 = vec1;
Vector<int> vec3 = std::move(vec1);
```
#### Основные возможности:
Swap - обмен двух векторов.
```CPP
Vector<int> vec1(5);
Vector<int> vec2(10);
vec1.Swap(vec2);
```
PushBack - добавление элементов в конец вектора.

PopBack - удаление элементов в конце вектора.
```CPP
Vector<int> vec;
vec.PushBack(1);
vec.PushBack(2);
vec.PushBack(3);
vec.PushBack(4);
vec.PopBack();
```
Insert - добавление элемента в произвольное место вектора (по итератору).

Erase - удаление элемента в произвольном месте вектора (по итератору).
```CPP
Vector<int> a;
a.Insert(a.cbegin(), 1);
a.Insert(a.cbegin(), 2);
a.Insert(a.cbegin(), 3);
a.Erase(a.begin() + 1);
std::cout << a;
```
Reserve - резервирование места в векторе под новые элементы.
```CPP
Vector<int> a;
std::cout << a.Capacity() << " " << a.Size() << std::endl;
// без резервирования при превышении размера будет происходить реаллокация
a.PushBack(1);
std::cout << a.Capacity() << " " << a.Size() << std::endl;
a.PushBack(2);
std::cout << a.Capacity() << " " << a.Size() << std::endl;
a.PushBack(3);
std::cout << a.Capacity() << " " << a.Size() << std::endl;
a.PushBack(4);
std::cout << a.Capacity() << " " << a.Size() << std::endl;

Vector<int> b;
b.Reserve(10);
std::cout << a.Capacity() << " " << a.Size() << std::endl;
// с резервированием реаллокации происходить не будет, т.к. места достаточно для размещения новых элементов
b.PushBack(1);
std::cout << b.Capacity() << " " << b.Size() << std::endl;
b.PushBack(2);
std::cout << b.Capacity() << " " << b.Size() << std::endl;
b.PushBack(3);
std::cout << b.Capacity() << " " << b.Size() << std::endl;
b.PushBack(4);
std::cout << b.Capacity() << " " << b.Size() << std::endl;
```
Resize - изменение размера вектора.
```CPP
Vector<int> a;
a.PushBack(1);
a.PushBack(2);
a.PushBack(3);
a.PushBack(4);
// при увеличении размера вектора новые элементы будут добавлены в конец вектора со значениями по умолчанию
a.Resize(10);
std::cout << a.Capacity() << " " << a.Size() << std::endl;
// при уменьшении размера вектора лишние элементы будут удалены, при этом ёмкость вектора не уменьшится
a.Resize(3);
std::cout << a.Capacity() << " " << a.Size() << std::endl;
```
Итерирование по элементам вектора в for-range цикле.
```CPP
Vector<int> a(10);
int i = 0;
// присваивем элементам вектора значения 0..9
for (auto &elem : a) {
    elem = i++;
}
```
## Использование
Скопируйте файлы vector.h и array_ptr.h в папку с вашим проектом и подключите через директиву #include<simple_vector.h>
## Системные требования
Компилятор С++ с поддержкой стандарта C++17 или новее
