## When updated code

```
git pull origin master:master
cd build
sudo make install -j4
```

## When updated code in submodule
```
git submodule foreach git pull origin master
```
