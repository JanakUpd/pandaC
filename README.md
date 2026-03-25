# Welcome to PandaC!

## Features
- **Easy to Learn**: Just like Python!
- **High Performance**: PandaC is compiled to machine code, allowing it to run at high speeds, making it suitable for performance-critical applications.
- **Cross-Platform**: PandaC can be run on various platforms, including Windows, macOS, and Linux(tested on Arch Linux and Ubuntu).

## Getting Started
To get started with PandaC, follow these steps:
## Linux/MacOS
1. Clone the repository:
   ```bash
   git clone https://github.com/JanakUpd/pandaC
    cd pandaC
    ```
2. Build the project using CMake:
    ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```
3. You are ready to go! - just launch the executable:
    ```bash
   ./pandaC -f="absolute/path/to/your/file.pc" --debug --no-execution
   ```
    --debug and --no-execution are optional flags that can be used for debugging purposes
## Examples of code in PandaC
1. Hello World:
```PandaC
using pandaC

def main():
   print("Greetings to the world of PandaC!")
   ```
2. Working with inputs:
```PandaC
using pandaC

def main():
    print("--- TYPE CONVERSION TEST ---")

    # Строка в число
    s_num = "125"
    real_num = int(s_num)
    real_num += 5
    print("String to int:", real_num) # Должно быть 130

    # Строка во float
    pi_str = "3.14"
    pi_num = float(pi_str)
    print("String to float:", pi_num)

    # Число в строку
    age = 20
    message = "I am " + str(age) + " years old"
    print("Int to string:", message)

    # Длина строк и массивов
    name = "Janak"
    print("Length of name:", len(name))

    arr = [1, 2, 3, 4, 5]
    print("Length of array:", len(arr))
   ```
3. Machine Learning Example:
```PandaC
using pandaC
using PandaML

def main()
    print("ML Trash Library Test")
    Matr<fl2> x_train
    x_train.data = {{1.0}, {2.0}, {3.0}, {4.0}, {5.0}}

    Array<fl2> y_train
    y_train.data = {3.0, 5.0, 7.0, 9.0, 11.0}

    LinearRegression model(x_train.cols())
    print("Training model on " + x_train.cols() + " samples...")
    model.fit(x_train.data, y_train.data)

    Array<fl2> unknown
    unknown.data = {10.0}
    fl2 prediction = model.predict(unknown.data)

    fl2 error = prediction - 21.0

    if error * error < 0.001:
        print("[PASS] Prediction is accurate.")
    else:
        print("[FAIL] Prediction is inaccurate.")
```
4. Machine Learning Example #2:
```PandaC
using pandaC
using PandaML

def main()
    print("KNN Test")
    Matr<fl2> X_train
    X_train.data = {{1.0, 1.0},{1.0, 2.0},{5.0, 5.0},{5.0, 6.0}}
    Array<fl2> y_train
    y_train.data = {0.0, 0.0, 1.0, 1.0}

    KNNClassifier model(3)
    print("Training KNN model...")
    model.fit(X_train.data, y_train.data)

    Array<fl2> unknown
    unknown.data = {6.0, 6.0}

    fl2 prediction = model.predict(unknown.data)
    print("Predicted class: " + prediction)

    if prediction == 1.0:
        print("[PASS] KNN Classification correct.")
    else:
        print("[FAIL] KNN Classification incorrect.")
```

## Libraries
**You can develop a library for your needs in c++ - you need just a .cpp file with the code you are planning to use and specify cpp dependencies in config file! You can take PandaML or standard library pandaC as an example.**
**Also if you really need to add keywords crucial for your code, you can add it via config/default.conf**

## Contributing
### The authors are:
- JanakUpd(Zhurikho Ivan) - [GitHub](https://github.com/JanakUpd)
- MoonInfinity2007(Kukoviakin Artem) - [GitHub](https://github.com/MoonInfinity2007)
- anoise1(Shumilo Andrew) - [GitHub](https://github.com/anoise1)
