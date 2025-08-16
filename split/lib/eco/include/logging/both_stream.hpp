/*
   Copyright 2022, Adam Krzywaniak.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#include <iostream>
#include <fstream>

class BothStream
{
public:
    BothStream() = delete;
    BothStream(BothStream&) = delete;
    BothStream(const BothStream&) = delete;
    virtual ~BothStream() = default;
    BothStream (std::ofstream& f) :
        fstr_(std::move(f)) {}

    template <class T>
    BothStream& operator<<(const T& obj) {
        std::cout << obj;
        fstr_ << obj;
        return *this;
    }
    BothStream& flush() {
        std::cout << std::flush;
        fstr_ << std::flush;
        return *this;
    }
private:
    std::ofstream fstr_;
};