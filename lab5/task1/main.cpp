#include <iostream>
#include <fstream>
#include <array>
#include <string>
#include <vector>
#include <optional>

enum class Gender
{
    Boy,
    Girl
};

enum class StringSerializationError
{
    Success,
    CommaInStringField
};

enum class ErrorCode
{
    Success,
    FileOpenError,
    FileReadError,
    FileWriteError,
    SerializationError,
    DeserializationError
};

struct Person
{
    std::array<char, 20> name;
    int form;
    Gender gender;
};

StringSerializationError serializeString(std::string_view str, std::ostream& stream) {
    if (str.find(',') != std::string_view::npos) {
        std::cerr << "Ошибка сериализации строки: строковое поле содержит запятую." << std::endl;
        return StringSerializationError::CommaInStringField;
    }
    stream << str;
    return StringSerializationError::Success;
}

void serializeInt(int value, std::ostream& stream)
{
    stream << value;
}

void serializeGender(Gender gender, std::ostream& stream)
{
    switch (gender)
    {
    case Gender::Boy:
        stream << "B,";
        break;
    case Gender::Girl:
        stream << "G,";
        break;
    }
}

void serializePerson(const Person& person, std::ostream& stream)
{
    serializeString(std::string_view(person.name.data()), stream);
    serializeInt(person.form, stream);
    serializeGender(person.gender, stream);
    stream << '\n';
}

int deserializeInt(const char* buffer) {
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

Gender deserializeGender(std::string_view buffer)
{
    if (buffer[0] == 'B')
        return Gender::Boy;
    else
        return Gender::Girl;
}

struct DeserializationResult
{
    bool success;
    Person value;
};

class PersonDeserializer
{
public:
    std::optional<Person> deserialize(std::string_view line) const
    {
        Person person;
        size_t pos = 0;

        size_t commaPos = line.find(',', pos);
        if (commaPos == std::string_view::npos)
            return std::nullopt;
        
        std::string nameStr(line.substr(pos, commaPos - pos));

        if (serializeString(nameStr, std::cerr) != StringSerializationError::Success)
            return std::nullopt;

        std::copy(nameStr.begin(), nameStr.end(), person.name.begin());
        person.name[nameStr.size()] = '\0';
        pos = commaPos + 1;

        commaPos = line.find(',', pos);
        if (commaPos == std::string::npos)
            return std::nullopt;
        person.form = deserializeInt(line.substr(pos).data());
        pos = commaPos + 1;

        person.gender = deserializeGender(line.substr(pos));
        return person;
    }
};

void serializePeople(const std::vector<Person>& people, std::ostream& stream)
{
    for (const auto& person : people)
    {
        serializePerson(person, stream);
        std::cout << std::endl;
    }
}

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

std::vector<Person> deserializePeople(std::istream& stream)
{
    std::vector<Person> people;
    std::string line;

    while (std::getline(stream, line))
    {
        PersonDeserializer deserializer;
        std::optional<Person> deserializedPerson = deserializer.deserialize(line);

        if (deserializedPerson)
        {
            people.push_back(*deserializedPerson);
        }
        else
        {
            std::cerr << "Ошибка десериализации строки: " << line << std::endl;
        }
    }

    return people;
}

Person generateRandomPerson()
{
    Person person;

    const char* charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    for (char& c : person.name)
    {
        c = charset[rand() % (sizeof(charset) - 1)];
    }
    person.name[19] = '\0';

    person.form = rand() % 12 + 1;

    person.gender = static_cast<Gender>(rand() % 2);

    return person;
}

int main()
{
    std::ofstream outFile("data.csv");
    ErrorCode writeError = checkFileWrite(outFile);
    if (writeError != ErrorCode::Success)
    {
        return static_cast<int>(writeError);
    }

    for (int i = 0; i < 5; ++i)
    {
        Person randomPerson = generateRandomPerson();
        serializePerson(randomPerson, outFile);
    }

    outFile.close();

    std::cout << "Рандомные данные сгенерировались и внесены в data.csv." << std::endl;

    Person person1 = { "Bebrina", 10, Gender::Girl };
    Person person2 = { "Megaman", 12, Gender::Boy };

    serializePeople({ person1, person2 }, outFile);

    outFile.close();

    std::ifstream inFile("data.csv");
    ErrorCode readError = checkFileRead(inFile);
    if (readError != ErrorCode::Success)
    {
        return static_cast<int>(readError);
    }

    auto people = deserializePeople(inFile);

    if (!people.empty())
    {
        people[0].form = 1;
        Person newPerson = { "Vasilisk", 5, Gender::Boy };
        people.push_back(newPerson);
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
