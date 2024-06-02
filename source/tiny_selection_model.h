//------------------------------------------------------------------------
// Copyright(c) 2024 Max And Me.
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
        this->selection_data = data;
        if (on_select_func)
            on_select_func(this->selection_data);
    }

    auto get_data() const -> const DataType& { return selection_data; }

    FuncOnSelect on_select_func;

    //------------------------------------------------------------------------
private:
    DataType selection_data{0};
};

//------------------------------------------------------------------------

} // namespace mam
