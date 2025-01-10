#include <iostream>

struct Animal {
protected:
    bool isSmile_ = false;

public:
    // 2 New Methods
    void say () {}
    void method () {}

public:
    // 1 attribute Smile
    void setSmile(bool isSmile) { isSmile_ = isSmile; }
    bool getSmile() { return isSmile_; }
};

struct Cat : Animal {
private:
    double beautyFactor_ = 0.0;

public:
    // Inherited overriden method
    void say () { std::cout << "Meow" << std::endl; }

public:
    // 1 inherited Attributes
    // 1 new Attribute
    void setBeautyFactor(int factor) { beautyFactor_ = factor; }
    int getBeautyFactor() { return beautyFactor_; }

private:
    // 1 New Hidden Method
    void PissInSlippers() { setSmile(true); }
};

struct Tiger : Animal {
public:
    // 1 overriden attribute
    void setSmile(bool isSmile) { isSmile ? sayPretty() : sayAngry(); }
    bool getSmile() { return isSmile_; }

private:
    // 1 New Hidden attribute
    void setSick(bool isSick) {};
    bool getSick() { return false; };

private:
    void sayAngry() {}
    void sayPretty() {}
};

struct Human {
public:
    Cat* pet;

private:
    void PetTheKitty() { pet->setSmile(true); }
};
