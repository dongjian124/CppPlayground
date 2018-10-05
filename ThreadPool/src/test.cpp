//
// Created by sb on 18-10-5.
//

#include "threadpool.h"
#include <bits/stdc++.h>

using namespace std;

int main()
{
    ThreadPool pool(4);

    vector<future<string>> res;

    for (int i = 0; i < 1000; ++i)
    {
        res.emplace_back(pool.AddTask([i]
                                      {
                                            return "Hello " + to_string(i);
                                      }));
    }

    for(auto&& r : res)
        cout << r.get() << endl;

}