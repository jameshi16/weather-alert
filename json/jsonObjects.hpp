#ifndef JSONOBJECTS_H
#define JSONOBJECTS_H

#include <string>
#include <vector>
#include <map>
#include <stdexcept>

// JsonObjects. They can have other objects in them, or lists and arrays.
// They are not self-aware, so code carefully.
template<typename S>
struct JsonObject {
    JsonObject& operator[](std::size_t id) {
        return objects[id];
    }

    JsonObject& operator[](std::string _name) {
        for (auto&& it : objects) { //tries to find the matching name in the object
            if (it.name == name)
                return it;
        }

        //if it reaches here, throw out_of_range error
        throw std::out_of_range("Name not found in this JsonObject");
    }

    std::vector<S> array; //contains elements, which are stored as strings
    std::string name; //the pair name
    std::string value; //the pair value (if it is reduceable)
    std::vector<JsonObject<S>> objects; //contains other objects that may be nested within
    
    JsonObject *parent = nullptr; //the pointer to the parent.
};

// A decode error, which is basically a runtime_error.
struct jsonDecodeError : std::runtime_error {
    jsonDecodeError(std::string errorMessage) : std::runtime_error(errorMessage){}
};

#endif