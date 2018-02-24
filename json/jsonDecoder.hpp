#ifndef JSONDECODER_H
#define JSONDECODER_H

#include "jsonObjects.hpp"
#include <vector>

/* A Json Interpreter. Not the best implementation you will see,
but I don't care
*/
class JsonDecoder {
    public:
    /* Constructors & Destructors */
    JsonDecoder()=default;
    virtual ~JsonDecoder()=default;
    JsonDecoder(const JsonDecoder&)=default;
    JsonDecoder(JsonDecoder&&)=default;

    JsonDecoder(std::string string_to_decode);

    JsonDecoder& operator=(const JsonDecoder&)=default;
    JsonDecoder& operator=(JsonDecoder&&)=default;

    /* Equality */
    bool operator==(const JsonDecoder& other) const;

    /* Access */
    JsonObject<std::string>& operator[](std::size_t id);
    JsonObject<std::string>& operator[](std::string name);

    /* Function to Decode */
    void decode(std::string string_to_decode);

    private:
    JsonObject<std::string> parentObject; //the parent JsonObject.
};

#endif