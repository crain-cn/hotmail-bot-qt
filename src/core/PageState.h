#pragma once

#include <QString>

enum class SignupPageState {
    Unknown,
    Email,
    Password,
    Profile,
    Birthdate,
    Otp,
    Captcha,
    Done
};

QString signupPageStateName(SignupPageState state);
