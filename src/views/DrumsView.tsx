import { useState } from 'react'
import type { CSSProperties } from 'react'
import { useStore } from '../state/store'
import { GRP, DRUM_KEYMAP } from '../state/seed'
import { panelCard, sectionLabel, colHd } from '../theme/tokens'
import { Toggle } from '../components/Toggle'

const COLS = '6px 30px 150px 52px 1fr 56px 56px 90px 50px 46px'
const msBtn = (on: boolean): CSSProperties => ({ font: "600 9px 'IBM Plex Mono'", width: 20, height: 18, display: 'flex', alignItems: 'center', justifyContent: 'center', borderRadius: 3, cursor: 'pointer', color: on ? 'var(--keyA-ink)' : 'var(--silk)', background: on ? 'var(--keyA)' : 'var(--panel)', border: '1px solid ' + (on ? 'var(--keyA-b)' : 'var(--panel-b)') })

export function DrumsView() {
  const drums = useStore((s) => s.drums)
  const toggleDrumMute = useStore((s) => s.toggleDrumMute)
  const toggleDrumSolo = useStore((s) => s.toggleDrumSolo)
  const setDrumLevel = useStore((s) => s.setDrumLevel)
  const [authentic, setAuthentic] = useState(true)
  const [groupMutes, setGroupMutes] = useState<Record<string, boolean>>({ TOMS: true })

  return (
    <div>
      {/* sub-header */}
      <div style={{ display: 'flex', alignItems: 'center', gap: 12, padding: '9px 16px', borderBottom: '1px solid var(--head-b)' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: 12, background: 'var(--disp)', border: '1px solid var(--disp-b)', borderRadius: 4, padding: '6px 14px', boxShadow: 'inset 0 1px 5px rgba(0,0,0,.3)' }}>
          <div style={{ font: "500 12px 'IBM Plex Mono'", color: 'var(--disp-dim)' }}>KIT</div>
          <div style={{ font: "500 14px 'IBM Plex Mono'", color: 'var(--disp-ink)' }}>RX21 STANDARD</div>
          <div style={{ font: "500 9px 'IBM Plex Mono'", color: 'var(--disp-ink)', border: '1px solid var(--disp-b)', borderRadius: 3, padding: '1px 5px' }}>9 SLOTS</div>
        </div>
        <div style={{ display: 'flex', alignItems: 'center', gap: 8, background: 'var(--panel)', border: '1px solid var(--panel-b)', borderRadius: 5, padding: '6px 12px' }}>
          <div style={{ font: "600 10px 'Barlow Condensed'", letterSpacing: '.12em', color: 'var(--silk)' }}>AUTHENTIC '85</div>
          <Toggle on={authentic} onToggle={() => setAuthentic((v) => !v)} />
          <div style={{ font: "400 9px 'IBM Plex Mono'", color: 'var(--faint)' }}>12-BIT</div>
        </div>
        <div style={{ flex: 1 }} />
        <div style={{ display: 'flex', alignItems: 'center', gap: 8, flexWrap: 'wrap' }}>
          <div style={{ ...sectionLabel, marginRight: 2 }}>GROUPS</div>
          {Object.keys(GRP).map((g) => {
            const on = !!groupMutes[g]
            return (
              <div key={g} style={{ display: 'flex', alignItems: 'center', gap: 6, background: 'var(--panel)', border: '1px solid var(--panel-b)', borderRadius: 4, padding: '4px 9px' }}>
                <div style={{ width: 8, height: 8, borderRadius: 2, background: GRP[g], flex: 'none' }} />
                <div style={{ font: "600 10px 'Barlow Condensed'", letterSpacing: '.1em', color: 'var(--val)' }}>{g}</div>
                <div onClick={() => setGroupMutes((m) => ({ ...m, [g]: !m[g] }))} style={msBtn(on)}>M</div>
              </div>
            )
          })}
        </div>
      </div>

      {/* slot table */}
      <div style={{ display: 'flex', flexDirection: 'column', gap: 4, padding: '10px 14px' }}>
        <div style={{ display: 'grid', gridTemplateColumns: COLS, gap: 10, alignItems: 'center', padding: '0 10px' }}>
          <div /><div style={colHd}>Nº</div><div style={colHd}>PATCH</div><div style={colHd}>KEY</div><div style={colHd}>LEVEL</div><div style={colHd}>PAN</div><div style={colHd}>TUNE</div><div style={colHd}>GROUP</div><div style={colHd}>CHOKE</div><div style={colHd}>M / S</div>
        </div>
        {drums.map((d, i) => (
          <div key={i} style={{ display: 'grid', gridTemplateColumns: COLS, gap: 10, alignItems: 'center', ...panelCard, borderRadius: 5, padding: '6px 10px' }}>
            <div style={{ width: 3, alignSelf: 'stretch', borderRadius: 2, background: GRP[d.grp], flex: 'none' }} />
            <div style={{ font: "500 10px 'IBM Plex Mono'", color: 'var(--faint)' }}>{d.n}</div>
            <div style={{ font: "500 11px 'IBM Plex Mono'", color: 'var(--disp-ink)', background: 'var(--disp)', border: '1px solid var(--disp-b)', borderRadius: 3, padding: '3px 8px', boxShadow: 'var(--disp-inset)', whiteSpace: 'nowrap', overflow: 'hidden' }}>{d.name}</div>
            <div style={{ font: "500 11px 'IBM Plex Mono'", color: 'var(--val)' }}>{d.key}</div>
            <div style={{ display: 'flex', alignItems: 'center', gap: 8 }}>
              <LevelBar value={d.lvl} onChange={(v) => setDrumLevel(i, v)} />
              <div style={{ font: "500 10px 'IBM Plex Mono'", color: 'var(--val)', width: 20, textAlign: 'right' }}>{d.lvl}</div>
            </div>
            <div style={{ font: "500 11px 'IBM Plex Mono'", color: 'var(--val)' }}>{d.pan}</div>
            <div style={{ font: "500 11px 'IBM Plex Mono'", color: 'var(--val)' }}>{d.tune}</div>
            <div style={{ font: "600 9.5px 'Barlow Condensed'", letterSpacing: '.1em', color: 'var(--silk)' }}>{d.grp}</div>
            <div style={{ font: "600 8.5px 'IBM Plex Mono'", padding: '1px 5px', borderRadius: 3, color: d.choke ? 'var(--disp-ink)' : 'transparent', background: d.choke ? 'var(--disp)' : 'transparent', border: '1px solid ' + (d.choke ? 'var(--disp-b)' : 'transparent') }}>{d.choke}</div>
            <div style={{ display: 'flex', gap: 4 }}>
              <div onClick={() => toggleDrumMute(i)} style={msBtn(d.mute)}>M</div>
              <div onClick={() => toggleDrumSolo(i)} style={msBtn(d.solo)}>S</div>
            </div>
          </div>
        ))}
        <div style={{ display: 'flex', justifyContent: 'center', padding: 5 }}><div style={{ font: "400 9.5px 'IBM Plex Mono'", color: 'var(--faint)' }}>+ 22 EMPTY SLOTS · DROP A VOICE OR CLICK TO ASSIGN</div></div>
      </div>

      {/* keyboard strip */}
      <div style={{ display: 'flex', alignItems: 'flex-end', gap: 2, padding: '4px 16px 12px' }}>
        {Array.from({ length: 25 }, (_, i) => {
          const black = [1, 3, 6, 8, 10].includes(i % 12)
          const grp = DRUM_KEYMAP[i]
          return (
            <div key={i} style={{ flex: 1, height: 38, borderRadius: '0 0 3px 3px', position: 'relative', background: black ? 'var(--ink)' : 'var(--key)', border: '1px solid var(--key-b)', borderTop: grp ? '4px solid ' + GRP[grp] : '4px solid transparent', display: 'flex', alignItems: 'flex-end', justifyContent: 'center' }}>
              <div style={{ font: "500 8px 'IBM Plex Mono'", color: black ? 'var(--silk)' : 'var(--faint)', paddingBottom: 2 }}>{i % 12 === 0 ? 'C' + (1 + i / 12) : ''}</div>
            </div>
          )
        })}
      </div>
    </div>
  )
}

function LevelBar({ value, onChange }: { value: number; onChange: (v: number) => void }) {
  const onDown = (e: React.PointerEvent) => {
    const el = e.currentTarget as HTMLElement
    el.setPointerCapture(e.pointerId)
    useStore.getState().beginInteraction()
    const set = (clientX: number) => {
      const rect = el.getBoundingClientRect()
      onChange(Math.round(Math.max(0, Math.min(1, (clientX - rect.left) / rect.width)) * 99))
    }
    set(e.clientX)
    const move = (ev: PointerEvent) => set(ev.clientX)
    const up = (ev: PointerEvent) => { el.releasePointerCapture(ev.pointerId); useStore.getState().endInteraction(); window.removeEventListener('pointermove', move); window.removeEventListener('pointerup', up) }
    window.addEventListener('pointermove', move)
    window.addEventListener('pointerup', up)
  }
  return (
    <div onPointerDown={onDown} style={{ flex: 1, height: 4, background: 'var(--inset)', borderRadius: 2, overflow: 'hidden', cursor: 'pointer', touchAction: 'none' }}>
      <div style={{ height: '100%', background: 'var(--acc)', width: Math.round((value / 99) * 100) + '%' }} />
    </div>
  )
}
