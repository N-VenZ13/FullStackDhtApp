<?php

use App\Http\Controllers\SensorController;
use Illuminate\Support\Facades\Route;

Route::get('/', function () {
    return view('welcome');
});

Route::get('/sensors', [SensorController::class, 'index']);
Route::get('/grafik', [SensorController::class, 'grafik']);
