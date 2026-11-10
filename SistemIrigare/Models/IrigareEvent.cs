namespace SistemIrigare.Models
{
    public class IrigareEvent
    {
        public int Id { get; set; }
        public DateTime TimpStart { get; set; }
        public DateTime? TimpStop { get; set; }
        public string Motiv { get; set; } = string.Empty; // "Automat" sau "Manual"
        public bool EsteActiv { get; set; }
    }

    public class SenzorDate
    {
        public int Id { get; set; }
        public DateTime Timestamp { get; set; }
        public float UmiditateSol { get; set; }
        public float Temperatura { get; set; }
        public float UmiditateAer { get; set; }
        public bool Ploaie { get; set; }
        public bool PompaActiva { get; set; }
    }
}