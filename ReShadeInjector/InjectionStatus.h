#pragma once

enum InjectionStatus : int
{
    SUCCESS = 0,
    FAILED_OPENPROCESS,
    FAILED_PROCESSNOTFOUND,
    FAILED_LOADLIBRARYFOUND
};
