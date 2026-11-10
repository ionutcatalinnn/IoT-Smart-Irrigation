using Microsoft.AspNetCore.SignalR;

namespace SistemIrigare.Hubs
{
    public class IrigareHub : Hub
    {
        // Metoda apelata de interfata web pentru a trimite comenzi către ESP32
        public async Task SchimbaStarePompa(bool stareNoua)
        {
            // Salvam comanda de la utilizator în Controller
            Controllers.SenzorController.ComandaPompaWeb = stareNoua;
            Controllers.SenzorController.AreComandaNoua = true;

            await Clients.All.SendAsync("ComandaPompa", stareNoua);
        }

        // Functii pentru Mod Auto si Oprire Urgenta ---
        public async Task SchimbaModAuto(bool esteAuto)
        {
            Controllers.SenzorController.ModAutoWeb = esteAuto;
            await Clients.All.SendAsync("ModAutoSchimbat", esteAuto);
        }

        public async Task OpresteUrgenta()
        {
            // Trecem sistemul pe manual si oprim pompa instant
            Controllers.SenzorController.ModAutoWeb = false;
            Controllers.SenzorController.ComandaPompaWeb = false;
            Controllers.SenzorController.AreComandaNoua = true;

            await Clients.All.SendAsync("ModAutoSchimbat", false);
            await Clients.All.SendAsync("ComandaPompa", false);
        }
    }
}