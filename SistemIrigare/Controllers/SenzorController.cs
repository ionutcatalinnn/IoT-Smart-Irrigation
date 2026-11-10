using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.SignalR;
using Microsoft.EntityFrameworkCore;
using SistemIrigare.Data;
using SistemIrigare.Hubs;
using SistemIrigare.Models;

namespace SistemIrigare.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class SenzorController : ControllerBase
    {
        private readonly AppDbContext _context;
        private readonly IHubContext<IrigareHub> _hubContext;

        
        public static bool AreComandaNoua = false;
        public static bool ComandaPompaWeb = false;
        public static bool ModAutoWeb = true; //  Implicit, sistemul e pe Automat
        // 

        public SenzorController(AppDbContext context, IHubContext<IrigareHub> hubContext)
        {
            _context = context;
            _hubContext = hubContext;
        }

        [HttpPost]
        public async Task<IActionResult> PostDateSenzor([FromBody] SenzorDate date)
        {
            // Ora inregistrarii in baza de data
            date.Timestamp = DateTime.Now;

            // Salveaza in baza de date
            _context.SenzoriDate.Add(date);

            //  Gestionare evenimente de irigare ---
            // Verificăm dacă exista un eveniment activ in baza de date
            var eventActiv = await _context.IrigareEvents
                .FirstOrDefaultAsync(e => e.EsteActiv);

            if (date.PompaActiva && eventActiv == null)
            {
                // Pompa tocmai a pornit — cream un eveniment nou
                string motiv = ModAutoWeb ? "Automat" : "Manual";
                _context.IrigareEvents.Add(new IrigareEvent
                {
                    TimpStart = DateTime.Now,
                    Motiv = motiv,
                    EsteActiv = true
                });
            }
            else if (!date.PompaActiva && eventActiv != null)
            {
                // Pompa tocmai s-a oprit — inchidem evenimentul existent
                eventActiv.TimpStop = DateTime.Now;
                eventActiv.EsteActiv = false;
            }

            await _context.SaveChangesAsync();

            //  Trimite datele instant cqtre interfata web (Dashboard)
            await _hubContext.Clients.All.SendAsync("PrimesteDateLive", date);

            //   RASPUNSUL PENTRU ESP32 ---
            //  Trimitem ESP-ului starea butonului si modul
            var raspuns = new { 
                areComandaNoua = AreComandaNoua, 
                comandaPompa = ComandaPompaWeb,
                modAuto = ModAutoWeb // Pachetul cu modul Auto/Manual
            };
            
            // Dupa ce ESP-ul preia comanda, o resetam pentru a nu o trimite la infinit
            AreComandaNoua = false; 

            return Ok(raspuns);
        }
    }
}