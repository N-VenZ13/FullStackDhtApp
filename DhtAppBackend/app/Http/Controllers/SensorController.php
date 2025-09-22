<?php

namespace App\Http\Controllers;

use App\Models\Sensor;
use Illuminate\Http\Request;

class SensorController extends Controller
{
    public function store(Request $request)
    {
        // Validasi input dulu
    $validated = $request->validate([
        'sensor_a' => 'required|numeric',
        'sensor_b' => 'required|numeric',
    ]);

    // Simpan ke database
    $sensor = Sensor::create($validated);

    return response()->json([
        'message' => 'Data saved successfully',
        'data' => $sensor
    ], 201);

    }

    public function index()
    {
        $data = Sensor::latest()->first();
        return view('sensors', compact('data'));
    }

    public function apiLatest()
    {
        return Sensor::latest()->first();
    }

    public function grafik()
    {
        return view('grafik');
    }

}
