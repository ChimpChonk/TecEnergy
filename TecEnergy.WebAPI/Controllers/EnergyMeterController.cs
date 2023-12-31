﻿using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using TecEnergy.Database.Models.DataModels;
using TecEnergy.Database.Models.DtoModels;
using TecEnergy.Database.Repositories;
using TecEnergy.Database.Repositories.Interfaces;
using TecEnergy.WebAPI.Services;

namespace TecEnergy.WebAPI.Controllers;
[Route("api/[controller]")]
[ApiController]
public class EnergyMeterController : ControllerBase
{
    //private readonly IEnergyMeterRepository _repository;
    private readonly EnergyMeterService _service;
    public EnergyMeterController(EnergyMeterService service)
    {
        _service = service;
    }

    [HttpGet]
    public async Task<ActionResult<IEnumerable<EnergyMeter>>> GetAllAsync()
    {
        var result = await _service.GetAllAsync();
        return Ok(result);
    }
    
    //SimpleDTO
    [HttpGet("{id}")]
    public async Task<ActionResult<EnergyMeter>> GetByIdAsync(Guid id)
    {
        var result = await _service.GetByIdAsync(id);
        if (result is null) return NotFound();
        return Ok(result);
    }

    [HttpGet("SimpleList/{roomId}")]
    public async Task<ActionResult<List<SimpleDto>>> GetSimpleEnergyMeterListByRoomId(Guid roomId)
    {
        var result = await _service.GetSimpleListByRoomIdAsync(roomId);
        if (result is null) return NotFound();
        return Ok(result);
    }

    //EnergyDTO
    [HttpGet("EnergyDto/{id}")]
    public async Task<ActionResult<EnergyDto>> GetDtoById(Guid id)
    {
        var startDateTime = DateTime.UtcNow;
        //if (endDateTime == null && startDateTime == null) endDateTime = DateTime.UtcNow; startDateTime = endDateTime.Value.AddSeconds(-60);
        var result = await _service.GetByIdDatetimeAsync(id, startDateTime, startDateTime.AddSeconds(-60));
        if(result is null) return NotFound();
        return Ok(result);
    }

    [HttpPost]
    public async Task<ActionResult<EnergyMeter>> CreateAsync(EnergyMeter energyMeter)
    {
        if (energyMeter is null) return BadRequest("Invalid input data.");
        await _service.CreateAsync(energyMeter);
        //return CreatedAtAction(nameof(GetByIdAsync), new { id = energyMeter.Id }, energyMeter);
        return Ok(energyMeter);
    }

    [HttpPut("{id}")]
    public async Task<IActionResult> PutAsync(Guid id, EnergyMeter updateResource)
    {
        if (id != updateResource.Id) return BadRequest(); 
        if (!ModelState.IsValid) return BadRequest(ModelState);
        var existingEnergyMeter = await _service.GetByIdAsync(id);
        if (existingEnergyMeter is null) return NotFound("EnergyMeter Not Found");
        await _service.UpdateAsync(id, updateResource);
        return NoContent();
    }

    [HttpDelete("{id}")]
    public async Task<IActionResult> DeleteAsync(Guid id)
    {
        await _service.DeleteAsync(id);
        return NoContent();
    }
}
