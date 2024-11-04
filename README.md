# Engine

<strong>Engine</strong> - простой движок на C++ использующий графический API OpenGL(glfw, glad) и систему сборки CMake.
<img src="https://github.com/user-attachments/assets/220a7437-ddf2-4154-9d7d-cb10ad7cef37">
<img src="https://upload.wikimedia.org/wikipedia/commons/1/13/Cmake.svg" width=100px>
<img src="https://upload.wikimedia.org/wikipedia/commons/1/18/ISO_C%2B%2B_Logo.svg" width=100px>
## Сборка
Для того чтобы собрать Engine, вам нужно сделать следующие шаги:

1. Скачать проект.
2. Зайти в папку проекта и ввести следующие команды:
	
	```
	cmake . -B build-tools
	```
	Это создаст папку build-tools где будут лежать инструменты сборки(проект Visual Studio или Makefile в зависимости от типа сборки).
	
	Далее введите:
    ```
    cmake --build build-tools
	```
    Эта команда соберет проект при помощи инструментов и поместит собранные файлы в папку build.
3. Откройте папку build, где будут лежать собранные файлы.

## Использование

Вы можете свободно использовать этот проект для разработки, а так же брать код для интеграции в свои проекты.

## Авторы

[Tosa](https://github.com/Tosa5656) - программист

## Contributing

Если вы желаете добавить свой код в проект, вы можете отправиьт свой pull request в наш проект, и мы рассмотрим его. Если ваш код будет хорошим дополнением для проекта, он будет добавлен в него.

## Материалы используемые в проекте

[GLFW](https://www.glfw.org/)  
[Glad](https://glad.dav1d.de/)  
[CMake](https://cmake.org/)

## Дополнительные материалы

[Learn OpenGL Habr](https://habr.com/ru/articles/310790/)    
[GLFW Documentation](https://www.glfw.org/docs/3.3/quick.html)