#include "jsonDecoder.hpp"

#include <stack>
#include <sstream>
#include <iostream>

JsonDecoder::JsonDecoder(std::string string_to_decode) : JsonDecoder() {
    decode(string_to_decode);
}

JsonObject<std::string>& JsonDecoder::operator[](std::size_t id) {
    return parentObject[id];
}

JsonObject<std::string>& JsonDecoder::operator[](std::string name) {
    return parentObject[name];
}

void JsonDecoder::decode(std::string string_to_decode) {
    using iterator_t = std::string::iterator;

    const std::string valueIndicators = "-0123456789.";
    /* Registers to use during the decoding of JSON */
    std::stack<iterator_t> objectBraces; //brace matching, end braces always close the last open brace.
    std::stack<iterator_t> arrayBraces; //brace matching, end braces always close the last open brace.
    std::stack<int> modes; //the modes. This defines what operation is taking place at the current level.
    //Enums are too troublesome in this case, so we use a mode instead. 0 - first run, 1 - dictionary, 2 - array, 3 - value, 4 - break character
    std::stringstream buffer; //the buffering stream.
    JsonObject<std::string>* cursor = &parentObject; //sets the cursor to the parent object.

    /* Start reading the JSON */
    for (iterator_t&& it = string_to_decode.begin(); it != string_to_decode.end(); std::advance(it, 1)) { //gets an iterator
        std::cerr << *it << ": Cursor Object Size: " << cursor->objects.size() << " Parent Object Size: " << parentObject.objects.size() << std::endl;
        /** Name-Value Detection **/ //Rule: It is a value until proven otherwise
        if (*it == '\"' && modes.size() && modes.top() != 3) { //open double quotes
            modes.push(3); //we start recognizing this as a value
            continue;
        }

        if (*it == '\"' && modes.size() && modes.top() == 3) { //close double quotes
            modes.pop(); //pops out the mode to the previous one, no longer recognize the following as a value
            continue;
        }

        if (valueIndicators.find_first_of(*it) != std::string::npos && modes.size() && modes.top() != 3) { //if a value is found, and no double quotes are in effect, treat it as a value too.
            buffer << *it; //writes the character into the stream
            continue; //next!
        }

        if (!modes.empty() && modes.top() == 3) {
            buffer << *it; //writes the character into the stream
            continue; //next character!
        }

        if (*it == ':' && modes.size() && modes.top() != 3) { //if not in a string, and colon is found
            cursor->objects.emplace_back(); //creates a new JsonObject at the back of the vector
            cursor->objects.back().parent = cursor; //sets the parent.

            cursor = &cursor->objects.back(); //cursor is now set to the newly created object
            buffer.flush(); //syncs the buffer
            cursor->name = buffer.str(); //puts the string into the name
            buffer.str(""); //clears the buffer
            continue; //aite next
        }

        if (*it == ',' && modes.size() && modes.top() == 2) { //if we see a comma and it's part of a list
            buffer.flush(); //flushes the buffer
            cursor->array.push_back(buffer.str()); //puts the string into the array
            buffer.str(""); //clears the buffer

            continue;
        }

        if (*it == ',' && modes.size() && modes.top() != 3) { //if we see a comma, and it's not part of a list
            buffer.flush(); //flushes the buffer
            cursor->value = buffer.str(); //puts the string into the value
            buffer.str(""); //clears the buffer

            cursor = cursor->parent; //goes back to the parent
            continue;
        }

        /** Open Brace Detection **/
        if (!modes.empty() && modes.top() == 3) //don't detect braces if currently in value mode
            continue;

        if (*it == '{' && !modes.size()) { /* Parent Object */
            objectBraces.push(it);
            cursor->parent = &parentObject; //the parent of the parent object is itself
            modes.push(1); //top-level operation is 1.
            continue;
        }

        if (*it == '{' && modes.size()) { /* Start of objects */
            objectBraces.push(it);
            cursor->objects.emplace_back(); //inserts a new object at the back of the parent object
            cursor->objects.back().parent = cursor; //sets the parent to the cursor json object
            cursor = &cursor->objects.back(); //sets the cursor
            modes.push(1); //next operation is object
            continue; //on to the next character
        }

        if (*it == '[' && modes.size()) { /* Start of array */
            arrayBraces.push(it);
            cursor->objects.emplace_back(); //inserts a new object at the back
            cursor->objects.back().parent = cursor;
            cursor = &cursor->objects.back(); //set the cursor
            modes.push(2); //adds the current mode
            continue; //on to the next character
        } else if (*it == '[' && !modes.size()) {
            throw jsonDecodeError("No parent object found.");
            break;
        }

        /** Close Brace Detection **/
        if (*it == '}') {
            if (objectBraces.empty()) { //if braces don't match
                throw jsonDecodeError("Object matching braces not found.");
                break;
            }

            objectBraces.pop();
            cursor->value = buffer.str();
            buffer.str("");
            cursor = cursor->parent; //goes back to the parent object of the cursor
            modes.pop(); //mode returns to parent mode
            continue;
        }

        if (*it == ']') {
            if (arrayBraces.empty()) {
                throw jsonDecodeError("List matching braces not found.");
                break;
            }

            arrayBraces.pop();
            cursor->value = buffer.str();
            buffer.str("");
            cursor = cursor->parent; //goes back to the parent object of the cursor
            modes.pop(); //mode returns to parent mode
            continue;
        }
    }
}