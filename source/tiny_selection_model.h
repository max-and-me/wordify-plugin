//------------------------------------------------------------------------
// Copyright(c) 2023 Max And Me.
//------------------------------------------------------------------------

#pragma once

namespace mam {

//------------------------------------------------------------------------
//  SelectionModel
//------------------------------------------------------------------------
template <typename T>
class SelectionModel
{
public:
    //------------------------------------------------------------------------
    using DataType     = T;
    using FuncOnSelect = std::function<void(const DataType& data)>;

    void select(const DataType& data)
    {
        this->data = data;
        if (on_select_func)
            on_select_func(this->data);
    }

    auto get_data() const -> const DataType& { return data; }

    FuncOnSelect on_select_func;

    //------------------------------------------------------------------------
private:
    DataType data{0};
};

//------------------------------------------------------------------------

} // namespace mam