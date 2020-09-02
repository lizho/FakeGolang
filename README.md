# FakeGolang
Make our C++ looks like the Golang.

The `defer` in golang is a good way to play with RAII, and `go` is another good job on asynchronous programming.

It's no doubt that we also needs them all in C++.

# Let's go.

```cpp
#include <iostream>
#include "fake_golang.h"

using namespace std;

template<class ...T>
static void atomic_print(T&&... args)
{
	static mutex m;
	lock_guard lock(m);
	((cout << forward<T>(args)), ...) << endl;
}


int main()
{
	lets_go;

	if (bool run = true)
	{
		defer[]{ atomic_print("done!"); };
		go[&run]
		{
			for (size_t i = 0; i < 10; i++)
			{
				atomic_print("go: ", i);
				this_thread::sleep_for(300ms);
			}
			run = false;
		};

		int n = 0;
		while (run)
		{
			atomic_print("waiting... ", n++);
			this_thread::sleep_for(500ms);
		}
	}

	return 0;
}

```

__A certain output:__
```
waiting... 0
go: 0
go: 1
waiting... 1
go: 2
go: 3
waiting... 2
go: 4
waiting... 3
go: 5
go: 6
waiting... 4
go: 7
go: 8
waiting... 5
go: 9
waiting... 6
done!
```
