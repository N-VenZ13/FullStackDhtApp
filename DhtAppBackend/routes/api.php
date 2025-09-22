<?php

use App\Http\Controllers\SensorController;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Route;

Route::get('/user', function (Request $request) {
    return $request->user();
})->middleware('auth:sanctum');

Route::post('/sensor/store', [SensorController::class, 'store']);
Route::get('/sensor/latest', [SensorController::class, 'apiLatest']);
