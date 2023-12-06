#include <iostream>
#include <fstream>
#include <array>
#include <string>
#include <vector>
#include <span>
#include <optional>


// Перечисление для пола
enum class Gender
{
    Boy,
    Girl
};

// Отдельный enum для ошибок сериализации строк
enum class StringSerializationError
{
    Success,
    CommaInStringField
};

// Отдельный enum для ошибок чтения и записи из файла
enum class ErrorCode
{
    Success,
    FileOpenError,
    FileReadError,
    FileWriteError,
    SerializationError,
    DeserializationError
};

// Структура данных
struct Person
{
    std::array<char, 20> name; // Строковое поле - имя ученика
    int form;                   // Целочисленное поле - класс, в котором находится ученик
    Gender gender;             // Пол - гендер ученика 
};

// Функция сериализации строки
StringSerializationError serializeString(std::string_view str, std::ostream& stream) {
    if (str.find(',') != std::string_view::npos) {

        // Обнаружена запятая в строке, сигнализируем об ошибке
        std::cerr << "Ошибка сериализации строки: строковое поле содержит запятую." << std::endl;
        return StringSerializationError::CommaInStringField;
    }

    stream << str; // Записываем строку без лишней запятой
    return StringSerializationError::Success;
}


// Функция сериализации целого числа
void serializeInt(int value, std::ostream& stream)
{
    stream << value; // Записываем целое число 
}

// Функция сериализации enum Gender
void serializeGender(Gender gender, std::ostream& stream)
{
    switch (gender)
    {
    case Gender::Boy:
        stream << "B,"; // Записываем "B" для Boy с последующей запятой
        break;
    case Gender::Girl:
        stream << "G,"; // Записываем "G" для Girl с последующей запятой
        break;
    }
}

// Функция сериализации объекта Person
void serializePerson(const Person& person, std::ostream& stream)
{
    serializeString(std::string_view(person.name.data()), stream); // Сериализация имени
    serializeInt(person.form, stream);                          // Сериализация целого числа
    serializeGender(person.gender, stream);                    // Сериализация enum Gender
    stream << '\n';                                           // Переход на новую строку после сериализации одного объекта
}

// Прототипы функций
int deserializeInt(const char* buffer);
Gender deserializeGender(std::string_view* buffer);


// Новая структура DeserializationResult для пункта 5
struct DeserializationResult
{
    bool success;
    Person value;
};

// Новый класс для десериализации Person
class PersonDeserializer
{
public:
    std::optional<Person> deserialize(std::string_view line) const // использую optional, т.к. мне кажется, что вместо того чтобы возвращать объект структуры с флагом успешности и значением, 
    // std::optional будет здесь более уместным.
    {
        Person person;
        size_t pos = 0;

        // Десериализация имени ученика
        size_t commaPos = line.find(',', pos);
        if (commaPos == std::string_view::npos)
            return std::nullopt;
        
        std::string nameStr = line.substr(pos, commaPos - pos);

        if (serializeString(nameStr, std::cerr) != StringSerializationError::Success) // cerrя
            return std::nullopt;

        std::copy(nameStr.begin(), nameStr.end(), person.name.begin());
        person.name[nameStr.size()] = '\0'; // Установка нулевого символа в конце строки
        pos = commaPos + 1;

        // Десериализация класса
        commaPos = line.find(',', pos);
        if (commaPos == std::string::npos)
            return std::nullopt;
        person.form = deserializeInt(line.substr(pos, commaPos - pos).c_str());
        pos = commaPos + 1;

        // Десериализация пола
        person.gender = deserializeGender(line.substr(pos).c_str());
        return person;
    }
};

// Функция десериализации строки
std::string_view deserializeString(const char* buffer, size_t length) {
    return std::string_view(buffer, length); // Возвращаем std::string_view из переданного буфера
}

// Функция десериализации целого числа
int deserializeInt(std::string_view buffer) {
    try {
        return std::stoi(std::string(buffer));
    } catch (const std::invalid_argument& e) {
        std::cerr << "Ошибка преобразования строки в число: " << e.what() << std::endl;
        return 0;
    } catch (const std::out_of_range& e) {
        std::cerr << "Ошибка: Число вне диапазона допустимых значений." << e.what() << std::endl;
        return 0;
    }
}

// Функция десериализации enum Gender
Gender deserializeGender(std::string_view buffer)
{
    if (buffer[0] == 'B')
        return Gender::Boy;   // Значения enum Gender для "B"
    else
        return Gender::Girl; // Значения enum Gender для "G"
}

/* Со span компилятор ругался, поэтому я решила сделать с вектором. Он вообще на очень многое ругался, особенно на ifstream/ofstream/iostream. Но я поисправляла)))
template <typename Span>
void serializePeople(const Span& people, std::ostream& stream)
{
    for (const auto& person : people)
    {
        serializePerson(person, stream); // Сериализация каждого объекта Person
        std::cout << std::endl; // Разделяю новой строчкой 
    }
}
*/

void serializePeople(const std::vector<Person>& people, std::ostream& stream)
{
    for (const auto& person : people)
    {
        serializePerson(person, stream); // Сериализация каждого объекта Person
        std::cout << std::endl; // Разделяю новой строчкой 
    }
}

// Функция для десериализации с использованием DeserializationResult
DeserializationResult deserialize(std::istream& stream)
{
    PersonDeserializer deserializer;

    std::string line;
    std::getline(stream, line);

    std::optional<Person> deserializedPerson = deserializer.deserialize(line);

    if (deserializedPerson)
    {
        return { true, *deserializedPerson };
    }
    else
    {
        return { false, {} };
    }
}

// Функция для дополнительных проверок на ошибки при чтении файла
ErrorCode checkFileRead(std::ifstream& file)
{
    if (!file.is_open())
    {
        std::cerr << "Ошибка открытия файла для чтения." << std::endl;
        return ErrorCode::FileOpenError;
    }

    if (file.fail())
    {
        std::cerr << "Ошибка чтения файла." << std::endl;
        return ErrorCode::FileReadError;
    }

    return ErrorCode::Success;
}

// Функция для дополнительных проверок на ошибки при записи файла
ErrorCode checkFileWrite(std::ofstream& file)
{
    if (!file.is_open())
    {
        std::cerr << "Ошибка открытия файла для записи." << std::endl;
        return ErrorCode::FileOpenError;
    }

    if (file.fail())
    {
        std::cerr << "Ошибка записи в файл." << std::endl;
        return ErrorCode::FileWriteError;
    }

    return ErrorCode::Success;
}

// Функция для десериализации vector объектов Person
std::vector<Person> deserializePeople(std::istream& stream)
{
    std::vector<Person> people;
    std::string line;

    // Читаем строку в потоке 
    while (std::getline(stream, line))
    {
        PersonDeserializer deserializer;
        std::optional<Person> deserializedPerson = deserializer.deserialize(line);

        // Проверяю, удалось ли десериализовать 
        if (deserializedPerson)
        {
            people.push_back(*deserializedPerson);
        }
        else
        {
            // Если десериализация не удалась, выводим сообщение об ошибке в поток ошибок
            std::cerr << "Ошибка десериализации строки: " << line << std::endl;
        }
    }

    return people; // Возврат вектора, содержащего десериализованные объекты Person
}

Person generateRandomPerson()
{
    Person person;

    // Генерация случайного имени
    const char* charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    for (char& c : person.name)
    {
        c = charset[rand() % (sizeof(charset) - 1)];
    }
    person.name[19] = '\0'; // Установка нулевого символа в конце строки

    // Генерация случайного класса (1-12)
    person.form = rand() % 12 + 1;

    // Генерация случайного пола
    person.gender = static_cast<Gender>(rand() % 2);

    return person;
}

int main()
{
    // Создание файла с сериализованными объектами
    std::ofstream outFile("data.csv");
    ErrorCode writeError = checkFileWrite(outFile);
    if (writeError != ErrorCode::Success)
    {
        return static_cast<int>(writeError);
    }

    // Генерация случайных данных и запись в файл
    for (int i = 0; i < 5; ++i) // Генерация 5 случайных записей
    {
        Person randomPerson = generateRandomPerson();
        serializePerson(randomPerson, outFile);
    }

    outFile.close();

    std::cout << "Рандомные данные сгенерировались и внесены в data.csv." << std::endl;

    Person person1 = { "Lera", 10, Gender::Girl };
    Person person2 = { "Vasea", 12, Gender::Boy };

    serializePeople({ person1, person2 }, outFile); // Сериализация объектов и запись в файл

    outFile.close();

    // Чтение из файла, модификация и запись обратно
    std::ifstream inFile("data.csv");
    ErrorCode readError = checkFileRead(inFile);
    if (readError != ErrorCode::Success)
    {
        return static_cast<int>(readError);
    }

    auto people = deserializePeople(inFile); // Десериализация данных из файла

    // Модификация данных
    if (!people.empty())
    {
        people[0].form = 1; // Изменение класса первого ученика
        Person newPerson = { "Vasilisk", 5, Gender::Boy };
        people.push_back(newPerson); // Добавление нового ученика
    }

    inFile.close();

    // Запись обновленных данных в файл
    std::ofstream newData("newData.csv");
    ErrorCode updateError = checkFileWrite(newData);
    if (updateError != ErrorCode::Success)
    {
        return static_cast<int>(updateError);
    }

    serializePeople(people, newData); // Сериализация обновленных данных и запись в новый файл

    newData.close();

    return 0;
}

/*Результат newData:
GBGFDCFAFGAFGGABGAE	1	B
DGFGFFGBBFBCAEFAEDD	3	G
FCAEBBEBAEDABEFBDEA	1	B
DDADGFACDGFAFGBCEED	11	B
BCEBAEAGEDEADAFGDEF	6	B
Petea	            5	B
*/
