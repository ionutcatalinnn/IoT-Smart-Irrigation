using System;
using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace SistemIrigare.Migrations
{
    /// <inheritdoc />
    public partial class InitialCreate : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.CreateTable(
                name: "IrigareEvents",
                columns: table => new
                {
                    Id = table.Column<int>(type: "INTEGER", nullable: false)
                        .Annotation("Sqlite:Autoincrement", true),
                    TimpStart = table.Column<DateTime>(type: "TEXT", nullable: false),
                    TimpStop = table.Column<DateTime>(type: "TEXT", nullable: true),
                    Motiv = table.Column<string>(type: "TEXT", nullable: false),
                    EsteActiv = table.Column<bool>(type: "INTEGER", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_IrigareEvents", x => x.Id);
                });

            migrationBuilder.CreateTable(
                name: "SenzoriDate",
                columns: table => new
                {
                    Id = table.Column<int>(type: "INTEGER", nullable: false)
                        .Annotation("Sqlite:Autoincrement", true),
                    Timestamp = table.Column<DateTime>(type: "TEXT", nullable: false),
                    UmiditateSol = table.Column<float>(type: "REAL", nullable: false),
                    Temperatura = table.Column<float>(type: "REAL", nullable: false),
                    UmiditateAer = table.Column<float>(type: "REAL", nullable: false),
                    Ploaie = table.Column<bool>(type: "INTEGER", nullable: false),
                    PompaActiva = table.Column<bool>(type: "INTEGER", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_SenzoriDate", x => x.Id);
                });
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropTable(
                name: "IrigareEvents");

            migrationBuilder.DropTable(
                name: "SenzoriDate");
        }
    }
}
